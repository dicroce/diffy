
#include <stdio.h>
#include <stdlib.h>
#include "XSDK/LargeFiles.h"
#include "XSDK/XHash.h"
#include "XSDK/XString.h"
#include "XSDK/XPath.h"
#include "XSDK/XMemoryMap.h"
#include "XSDK/LargeFiles.h"
#include "XSDK/XRef.h"
#include "MD5.h"

using namespace XSDK;
using namespace std;

void PopulateFileHash( const XString& path,
                       const XHash<XString>& ignores,
                       XHash<XString>& fileHash )
{
    XString fileName = path.substr( path.rfind( PATH_SLASH ) + 1 );
    if( ignores.Find( fileName ) )
        return;

    if( XPath::IsDir( path ) )
    {
        XPath files( path );

        for( files.IterStart(); files.IterValid(); files.Iterate() )
            PopulateFileHash( path + PATH_SLASH + *files.IterData(), ignores, fileHash );
    }
    else
    {
        fileHash.Add( fileName, path );
    }
}

XHash<XString> GetIgnores()
{
    XHash<XString> output;

    if( XPath::Exists( "/etc/diffy_ignores" ) )
    {
        FILE* inFile = fopen( "/etc/diffy_ignores", "r+" );

        if( !inFile )
            X_THROW(("Unable to open ignores file."));

        char lineBuffer[2048];

        while( !feof( inFile ) )
        {
            if( fgets( lineBuffer, 2048, inFile ) )
            {
                XString line = lineBuffer;

                line = line.Strip();

                output.Add( line, line );
            }
        }

        fclose( inFile );
    }

    return output;
}

bool Identical( const XSDK::XString& pathA, const XSDK::XString& pathB )
{
    if( !XPath::Exists( pathA ) )
        X_THROW(("Unable to open %s",pathA.c_str()));

    x_file_info pathAInfo;
    x_stat( pathA.c_str(), &pathAInfo );

    if( !XPath::Exists( pathB ) )
        X_THROW(("Unable to open %s",pathB.c_str()));

    x_file_info pathBInfo;
    x_stat( pathB.c_str(), &pathBInfo );

    if( (pathAInfo._fileSize == 0) &&
        (pathBInfo._fileSize == 0) )
        return true;

    if( pathAInfo._fileSize != pathBInfo._fileSize )
        return false;

    FILE* fileA = fopen( pathA.c_str(), "rb" );
    if( !fileA )
        X_THROW(("Unable to open %s",pathA.c_str()));

    FILE* fileB = fopen( pathB.c_str(), "rb" );
    if( !fileB )
        X_THROW(("Unable to open %s",pathB.c_str()));

    XRef<XMemoryMap> pathAMap = new XMemoryMap( fileno( fileA ),
                                                0,
                                                (uint32_t)pathAInfo._fileSize,
                                                XMemoryMap::XMM_PROT_READ,
                                                XMemoryMap::XMM_TYPE_FILE | XMemoryMap::XMM_SHARED );

    XRef<XMemoryMap> pathBMap = new XMemoryMap( fileno( fileB ),
                                                0,
                                                (uint32_t)pathBInfo._fileSize,
                                                XMemoryMap::XMM_PROT_READ,
                                                XMemoryMap::XMM_TYPE_FILE | XMemoryMap::XMM_SHARED );

    MD5_CTX pathAMD5;
    MD5_Init( &pathAMD5 );

    MD5_CTX pathBMD5;
    MD5_Init( &pathBMD5 );

    MD5_Update( &pathAMD5, pathAMap->Map(), pathAMap->Length() );
    MD5_Update( &pathBMD5, pathBMap->Map(), pathBMap->Length() );

    uint8_t pathAHash[16];
    memset( pathAHash, 0, 16 );

    uint8_t pathBHash[16];
    memset( pathBHash, 0, 16 );

    MD5_Final( pathAHash, &pathAMD5 );
    MD5_Final( pathBHash, &pathBMD5 );

    if( memcmp( pathAHash, pathBHash, 16 ) == 0 )
        return true;

    return false;
}

int main( int argc, char* argv[] )
{
#if 0
    {
        x_file_info inFileInfo;
        x_stat( argv[1], &inFileInfo );

        FILE* inFile = fopen( argv[1], "rb" );

        XRef<XMemoryMap> inFileMap = new XMemoryMap( fileno( inFile ),
                                                     0,
                                                     (uint32_t)inFileInfo._fileSize,
                                                     XMemoryMap::XMM_PROT_READ,
                                                     XMemoryMap::XMM_TYPE_FILE | XMemoryMap::XMM_SHARED );

        MD5_CTX inFileMD5;
        MD5_Init( &inFileMD5 );

        MD5_Update( &inFileMD5, inFileMap->Map(), inFileMap->Length() );

        uint8_t inFileHash[16];
        memset( inFileHash, 0, 16 );

        MD5_Final( inFileHash, &inFileMD5 );

        for( int i = 0; i < 16; i++ )
            printf("%02x",inFileHash[i]);
        printf("\n");
        fflush(stdout);
        exit(0);
    }
#endif




    if( argc < 3 )
    {
        printf("Invalid Args.\n");
        fflush(stdout);
        exit(0);
    }

    XHash<XString> ignores = GetIgnores();

    XHash<XString> fileHashA;
    PopulateFileHash( argv[1], ignores, fileHashA );

    XHash<XString> fileHashB;
    PopulateFileHash( argv[2], ignores, fileHashB );

    XHash<XString>::XHashIter iter;
    for( iter = fileHashA.GetIterator(); iter.IterValid(); iter.Iterate() )
    {
        XString key = iter.IterKey();
        XString sourcePath = *iter.IterData();

        if( fileHashB.Find( key ) )
        {
            XString destPath = *fileHashB.Find( key );

            if( !Identical( sourcePath, destPath ) )
            {
                XString cmd = XString::Format( "meld %s %s", sourcePath.c_str(), destPath.c_str() );
                system( cmd.c_str() );
            }
        }
    }

    return 0;
}
