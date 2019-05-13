//! See https://isatec.ca/fastcgipp/helloWorld.html
//! [Request definition]
#include <fastcgi++/request.hpp>
#include <iomanip>

class HelloWorld: public Fastcgipp::Request<wchar_t>
{
public:
    HelloWorld(HelloWorld&& rhs) = default;
    HelloWorld(Request&& rhs) :
        Request::Request(std::move(rhs))
    {
    }
    ~HelloWorld() {
        vlog("%s\n", __func__);
    }
    //! [Request definition]
    //! [Response definition]
    bool response()
    {
        vlog("%s\n", __PRETTY_FUNCTION__);

        //! [Response definition]
        //! [HTTP header]
        out << L"Content-Type: text/html; charset=utf-8\r\n\r\n";
        //! [HTTP header]

        //! [Output]
        out <<
L"<!DOCTYPE html>\n"
L"<html>"
    L"<head>"
        L"<meta charset='utf-8' />"
        L"<title>fastcgi++: Hello World</title>"
    L"</head>"
    L"<body>"
        L"<p>"
            L"English: Hello World<br>"
            L"Russian: Привет мир<br>"
            L"Greek: Γεια σας κόσμο<br>"
            L"Chinese: 世界您好<br>"
            L"Japanese: 今日は世界<br>"
            L"Runic English?: ᚺᛖᛚᛟ ᚹᛟᛉᛚᛞ<br>"
        L"</p>"
    L"</body>"
L"</html>";
        //! [Output]

        //! [Return]
        return true;
    }
};
//! [Return]

class Echo: public Fastcgipp::Request<wchar_t>
{
    //! [Request definition]
    //! [Max POST]
public:
    Echo():
        Fastcgipp::Request<wchar_t>(5*1024)
    {}
    Echo(Echo&& rhs) = default;
    Echo(Request&& rhs) :
        Request::Request(std::move(rhs))
    {
    }
    ~Echo() {
        vlog("%s\n", __func__);
    }
private:
    bool response()
    {
        vlog("%s\n", __PRETTY_FUNCTION__);

        //! [Max POST]
        //! [Encoding using]
        using Fastcgipp::Encoding;
        //! [Encoding using]

        //! [Header]
        out << L"Set-Cookie: echoCookie=" << Encoding::URL << L"<\"русский\">;"
            << Encoding::NONE << L"; path=/\n";
        out << L"Content-Type: text/html; charset=utf-8\r\n\r\n";
        //! [Header]

        //! [HTML]
        out <<
L"<!DOCTYPE html>\n"
L"<html>"
    L"<head>"
        L"<meta charset='utf-8' />"
        L"<title>fastcgi++: Echo</title>"
    L"</head>"
    L"<body>"
        L"<h1>Echo</h1>";
        //! [HTML]

        //! [Environment]
        out <<
        L"<h2>Environment Parameters</h2>"
        L"<p>"
            L"<b>FastCGI Version:</b> "
                << Fastcgipp::Protocol::version << L"<br />"
            L"<b>fastcgi++ Version:</b> " << Fastcgipp::version << L"<br />"
            L"<b>Hostname:</b> " << Encoding::HTML << environment().host
                << Encoding::NONE << L"<br />"
            L"<b>User Agent:</b> " << Encoding::HTML << environment().userAgent
                << Encoding::NONE << L"<br />"
            L"<b>Accepted Content Types:</b> " << Encoding::HTML
                << environment().acceptContentTypes << Encoding::NONE
                << L"<br />"
            L"<b>Accepted Languages:</b> " << Encoding::HTML;
        if(!environment().acceptLanguages.empty())
        {
            auto language = environment().acceptLanguages.cbegin();
            while(true)
            {
                out << language->c_str();
                ++language;
                if(language == environment().acceptLanguages.cend())
                    break;
                out << ',';
            }
        }
        out << Encoding::NONE << L"<br />"
            L"<b>Accepted Characters Sets:</b> " << Encoding::HTML
                << environment().acceptCharsets << Encoding::NONE << L"<br />"
            L"<b>Referer:</b> " << Encoding::HTML << environment().referer
                << Encoding::NONE << L"<br />"
            L"<b>Content Type:</b> " << Encoding::HTML
                << environment().contentType << Encoding::NONE << L"<br />"
            L"<b>Root:</b> " << Encoding::HTML << environment().root
                << Encoding::NONE << L"<br />"
            L"<b>Script Name:</b> " << Encoding::HTML
                << environment().scriptName << Encoding::NONE << L"<br />"
            L"<b>Request URI:</b> " << Encoding::HTML
                << environment().requestUri << Encoding::NONE << L"<br />"
            L"<b>Request Method:</b> " << environment().requestMethod
                << L"<br />"
            L"<b>Content Length:</b> " << environment().contentLength
                << L" bytes<br />"
            L"<b>Keep Alive Time:</b> " << environment().keepAlive
                << L" seconds<br />"
            L"<b>Server Address:</b> " << environment().serverAddress
                << L"<br />"
            L"<b>Server Port:</b> " << environment().serverPort << L"<br />"
            L"<b>Client Address:</b> " << environment().remoteAddress << L"<br />"
            L"<b>Client Port:</b> " << environment().remotePort << L"<br />"
            L"<b>Etag:</b> " << environment().etag << L"<br />"
            L"<b>If Modified Since:</b> " << Encoding::HTML
                << std::put_time(std::gmtime(&environment().ifModifiedSince),
                        L"%a, %d %b %Y %H:%M:%S %Z") << Encoding::NONE <<
        L"</p>";
        //! [Environment]

        //! [Path Info]
        out <<
        L"<h2>Path Info</h2>";
        if(environment().pathInfo.size())
        {
            out <<
        L"<p>";
            std::wstring preTab;
            for(const auto& element: environment().pathInfo)
            {
                out << preTab << Encoding::HTML << element << Encoding::NONE
                    << L"<br />";
                preTab += L"&nbsp;&nbsp;&nbsp;&nbsp;";
            }
            out <<
        L"</p>";
        }
        else
            out <<
        L"<p>No Path Info</p>";
        //! [Path Info]

        //! [Other Environment]
        out <<
        L"<h2>Other Environment Parameters</h2>";
        if(!environment().others.empty())
            for(const auto& other: environment().others)
                out << L"<b>" << Encoding::HTML << other.first << Encoding::NONE
                    << L":</b> " << Encoding::HTML << other.second
                    << Encoding::NONE << L"<br />";
        else
            out <<
        L"<p>No Other Environment Parameters</p>";
        //! [Other Environment]

        //! [GET Data]
        out <<
        L"<h2>GET Data</h2>";
        if(!environment().gets.empty())
            for(const auto& get: environment().gets)
                out << L"<b>" << Encoding::HTML << get.first << Encoding::NONE
                    << L":</b> " << Encoding::HTML << get.second
                    << Encoding::NONE << L"<br />";
        else
            out <<
        L"<p>No GET data</p>";
        //! [GET Data]

        //! [POST Data]
        out <<
        L"<h2>POST Data</h2>";
        if(!environment().posts.empty())
            for(const auto& post: environment().posts)
                out << L"<b>" << Encoding::HTML << post.first << Encoding::NONE
                    << L":</b> " << Encoding::HTML << post.second
                    << Encoding::NONE << L"<br />";
        else
            out <<
        L"<p>No POST data</p>";
        //! [POST Data]

        //! [Cookies]
        out <<
        L"<h2>Cookies</h2>";
        if(!environment().cookies.empty())
            for(const auto& cookie: environment().cookies)
                out << L"<b>" << Encoding::HTML << cookie.first
                    << Encoding::NONE << L":</b> " << Encoding::HTML
                    << cookie.second << Encoding::NONE << L"<br />";
        else
            out <<
        L"<p>No Cookies</p>";
        //! [Cookies]

        //! [Files]
        out <<
        L"<h2>Files</h2>";
        if(!environment().files.empty())
        {
            for(const auto& file: environment().files)
            {
                out <<
        L"<h3>" << Encoding::HTML << file.first << Encoding::NONE << L"</h3>"
        L"<p>"
            L"<b>Filename:</b> " << Encoding::HTML << file.second.filename
                << Encoding::NONE << L"<br />"
            L"<b>Content Type:</b> " << Encoding::HTML
                << file.second.contentType << Encoding::NONE << L"<br />"
            L"<b>Size:</b> " << file.second.size << L"<br />"
            L"<b>Data:</b>"
        L"</p>"
        L"<pre>";
                //! [Files]
                //! [Dump]
                dump(file.second.data.get(), file.second.size);
                out <<
        L"</pre>";
            }
        }
        else
            out <<
        L"<p>No files</p>";
        //! [Dump]

        //! [Form]
        out <<
        L"<h1>Form</h1>"
        L"<h3>multipart/form-data</h3>"
        L"<form action='?getVar=testing&amp;secondGetVar=tested&amp;"
            L"utf8GetVarTest=проверка&amp;enctype=multipart' method='post' "
            L"enctype='multipart/form-data' accept-charset='utf-8'>"
            L"Name: <input type='text' name='+= aquí está el campo' value='Él "
                L"está con un niño' /><br />"
            L"File: <input type='file' name='aFile' /> <br />"
            L"<input type='submit' name='submit' value='submit' />"
        L"</form>"
        L"<h3>application/x-www-form-urlencoded</h3>"
        L"<form action='?getVar=testing&amp;secondGetVar=tested&amp;"
            L"utf8GetVarTest=проверка&amp;enctype=url-encoded' method='post' "
            L"enctype='application/x-www-form-urlencoded' "
            L"accept-charset='utf-8'>"
            L"Name: <input type='text' name='+= aquí está el campo' value='Él "
                L"está con un niño' /><br />"
            L"File: <input type='file' name='aFile' /><br />"
            L"<input type='submit' name='submit' value='submit' />"
        L"</form>";
        //! [Form]

        //! [Finish]
        out <<
    L"</body>"
L"</html>";

        return true;
    }
};

//! [Manager]
#include <fastcgi++/manager.hpp>
#include <fastcgi++/log.hpp>
#include <signal.h>

int main()
{
    signal(SIGSEGV, faultHandler);
    signal(SIGABRT, faultHandler);
    //std::set_terminate(terminateHandler);

    //freopen("/dev/tty", "a", stderr);
    
    Fastcgipp::MultiUriRouter<wchar_t> manager;

    Fastcgipp::MultiUriRouter<wchar_t>::requestCreator f1(Fastcgipp::RequestCreator<wchar_t, HelloWorld>);
    //Fastcgipp::MultiUriRouter<wchar_t>::requestCreator f2(Fastcgipp::RequestCreator<wchar_t, Echo>);

	manager.routeUri(L"/vision2/hello", f1);
	manager.routeUri(L"/vision2/echo",  Fastcgipp::RequestCreator<wchar_t, Echo>);

    manager.setupSignals();
    manager.listen();
    manager.start();
    manager.join();

    return 0;
}
//! [Join]
