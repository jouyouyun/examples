// From: http://www.cppblog.com/arthaslee/archive/2010/12/01/135186.html
#include <iostream>
#include <string.h>

#include <cryptopp/sha.h>
#include <cryptopp/secblock.h>
#include <cryptopp/modes.h>
#include <cryptopp/hex.h>

using namespace std;
using namespace CryptoPP;

void CalculateDigest(string &Digest, const string &Message);
bool VerifyDigest(const string &Digest, const string &Message);

int main( void )
{
    //main函数中注释掉的，关于strMessage2的代码，其实是笔者模拟了一下
    //通过求Hash值来对“大”量数据进行校验的这个功能的运用。
    //注释之后并不影响这段代码表达的思想和流程。
    string strMessage( "Hello world" );
    string strDigest;
    //string strMessage2( "hello world" ); //只是第一个字母不同
    //string strDigest2;

    CalculateDigest( strDigest, strMessage );  //计算Hash值并打印一些debug信息
    cout << "the size of Digest is: " << strDigest.size() << endl;
    cout << "Digest is: " << strDigest << endl;

    //CalculateDigest( strDigest2, strMessage2 );
    //why put this function here will affect the Verify function?
    //作者在写代码的过程中遇到的上述问题。
    //如果把这行代码的注释取消，那么之后的运行结果就不是预料中的一样：
    //即使strDigest也无法对应strMessage，笔者不知道为什么，希望高手指出，谢谢！

    bool bIsSuccess = false;
    bIsSuccess = VerifyDigest( strDigest, strMessage );
    //通过校验，看看strDigest是否对应原来的message
    if( bIsSuccess )
    {
        cout << "sussessive verify" << endl;
        cout << "origin string is: " << strMessage << endl << endl;
    }
    else
    {
        cout << "fail!" << endl;
    }

    //通过strDigest2与strMessage进行校验，要是相等，
    //就证明strDigest2是对应的strMessage2跟strMessage1相等。
    //否则，像这个程序中的例子一样，两个message是不相等的
    /*CalculateDigest( strDigest2, strMessage2 );
    bIsSuccess = VerifyDigest( strDigest2, strMessage );
    if( !bIsSuccess )
    {
        cout << "success! the tiny modification is discovered~" << endl;
        cout << "the origin message is: \n" << strMessage << endl;
        cout << "after modify is: \n" << strMessage2 << endl;
    }*/
    return 0;
}


//基于某些原因，以下两个子函数的实现跟原来参考代码中的实现有所区别,
//详细原因，笔者在CalculateDigest函数的注释中写明
void CalculateDigest(string &Digest, const string &Message)
{
    SHA256 sha256;
    int DigestSize = sha256.DigestSize();
    char* byDigest;
    // char* strDigest;

    byDigest = new char[ DigestSize ];
    // strDigest = new char[ DigestSize * 2 + 1 ];

    sha256.CalculateDigest((byte*)byDigest, (const byte *)Message.c_str(), Message.size());
    // memset(strDigest, 0, sizeof(strDigest));
    //uCharToHex(strDigest, byDigest, DigestSize);
    //参考的代码中有以上这么一行，但是貌似不是什么库函数。
    //原作者大概是想把Hash值转换成16进制数保存到一个string buffer中，
    //然后在主程序中输出，方便debug的时候对照查看。
    //但是这并不影响计算Hash值的行为。
    //因此笔者注释掉了这行代码，并且修改了一下这个函数和后面的VerifyDigest函数，
    //略去原作者这部分的意图，继续我们的程序执行。

    Digest = byDigest;

    delete []byDigest;
    byDigest = NULL;
    // delete []strDigest;
    // strDigest = NULL;

    return;
}

bool VerifyDigest(const string &Digest, const string &Message)
{
    bool Result;
    SHA256 sha256;
    char* byDigest;

    byDigest = new char[ sha256.DigestSize() ];
    strcpy( byDigest, Digest.c_str() );

    //HexTouChar(byDigest, Digest.c_str(), Digest.size());
    //为何注释掉，请参看CalculateDigest函数的注释
    Result = sha256.VerifyDigest( (byte*)byDigest, (const byte *)Message.c_str(), Message.size() );

    delete []byDigest;
    byDigest = NULL;
    return Result;
}
