//! https://isatec.ca/fastcgipp/gnu.html
//! [Request definition]
#include <fastcgi++/request.hpp>
#include <iomanip>

class Gnu: public Fastcgipp::Request<wchar_t>
{
    //! [Request definition]
    //! [Locales declaration]
    static const std::vector<std::string> locales;
    //! [Locales declaration]
    //! [Catalogues declaration]
    static const std::vector<std::vector<std::wstring>> catalogues;
    //! [Catalogues declaration]
    //! [Image declaration]
    static const unsigned char gnuPng[];
    static const size_t gnuPngSize = 58587;
    //! [Image declaration]
    //! [startTime declaration]
    static const std::time_t startTimestamp;
    static const std::tm startTime;
    //! [startTime declaration]

    //! [Image output]
    inline void image()
    {
        //! [Image output]
        //! [Cached image]
		if(startTimestamp <= environment().ifModifiedSince)
			out << "Status: 304 Not Modified\r\n\r\n";
        //! [Cached image]
        //! [Uncached image]
        else
        {
            out << "Last-Modified: "
                << std::put_time(&startTime, L"%a, %d %b %Y %H:%M:%S GMT\n");
            out << L"Content-Length: " << gnuPngSize << '\n';
            out << L"Content-Type: image/png\r\n\r\n";
            dump(gnuPng, gnuPngSize);
        }
    }
    //! [Uncached image]

    //! [HTML output]
    inline void html()
    {
        //! [HTML output]
        //! [pickLocale]
        const unsigned locale = pickLocale(locales);
        const std::wstring language(
                locales[locale].cbegin(),
                locales[locale].cbegin()+2);
        const std::vector<std::wstring>& catalogue(catalogues[locale]);
        //! [pickLocale]

        //! [Cached HTML]
        if(
                locale==environment().etag
                && startTimestamp<=environment().ifModifiedSince)
			out << "Status: 304 Not Modified\r\n\r\n";
        //! [Cached HTML]
        //! [Uncached HTML]
        else
        {
            out << "Last-Modified: "
                << std::put_time(&startTime, L"%a, %d %b %Y %H:%M:%S GMT\n");
            out << L"Etag: " << locale << '\n';
            out << L"Content-Type: text/html; charset=utf-8\n";
            out << L"Content-Language: " << language << L"\r\n\r\n";
            //! [Uncached HTML]

            //! [setLocale]
            setLocale(locales[locale]);
            //! [setLocale]

            //! [Body output]
            out <<
L"<!DOCTYPE html>\n"
L"<html lang='" << language << L"'>"
    L"<head>"
        L"<meta charset='utf-8' />"
        L"<title>fastcgi++: " << catalogue[0] << L"</title>"
    L"</head>"
    L"<body>"
        L"<h1>" << catalogue[1] << L"</h1>"
        L"<figure>"
            L"<img src='" << environment().scriptName << L"/gnu.png' alt='"
                << catalogue[2] << L"'>"
            L"<figcaption>" << catalogue[3] << gnuPngSize
                << catalogue[4] << std::put_time(&startTime, L"%c")
                << L". </figcaption>"
        L"</figure>"
    L"</body>"
L"</html>";
        }
    }
    //! [Body output]

    //! [Response]
	bool response()
	{
        if(environment().requestUri.find(L"gnu.png") != environment().requestUri.npos)
            image();
        else
            html();

		return true;
	}
};
//! [Response]

//! [Definitions]
const std::vector<std::string> Gnu::locales
{
    "en_CA",
    "en_US",
    "fr_CA",
    "fr_FR",
    "zh_CN",
    "de_DE"
};

const std::vector<std::vector<std::wstring>> Gnu::catalogues
{
    {
        L"Showing the colourless GNU",
        L"This is a header",
        L"The GNU Logo",
        L"Figure 1: This GNU logo is ",
        L" bytes. It was last modified "
    },
    {
        L"Showing the colorless GNU",
        L"This is a header",
        L"The GNU Logo",
        L"Figure 1: This GNU logo is ",
        L" bytes. It was last modified "
    },
    {
        L"Montrant le GNU incolore",
        L"Ceci est un en-tête",
        L"Le logo GNU",
        L"Figure 1: Ce logo GNU est de ",
        L" octets. Il a été modifié "
    },
    {
        L"Montrant le GNU incolore",
        L"Ceci est un en-tête",
        L"Le logo GNU",
        L"Figure 1: Ce logo GNU est de ",
        L" octets. Il a été modifié "
    },
    {
        L"顯示無色GNU",
        L"這是一個標頭",
        L"GNU的標誌",
        L"圖1 ：這GNU標誌是",
        L"字節。最後一次修改"
    },
    {
        L"Angezeigt wird die farblose GNU",
        L"Dies ist ein Kopf",
        L"Das GNU- Logo",
        L"Abbildung 1: Das GNU -Logo ist ",
        L" Bytes. Es wurde zuletzt geändert "
    }
};

const unsigned char Gnu::gnuPng[] =
#include "gnu.png.hpp"

const std::time_t Gnu::startTimestamp = std::time(nullptr);
const std::tm Gnu::startTime = *std::gmtime(&startTimestamp);
//! [Definitions]

//! [Finish]
#include <fastcgi++/manager.hpp>

int main()
{
    Fastcgipp::Manager<Gnu> manager;
    manager.setupSignals();
    manager.listen();
    manager.start();
    manager.join();

    return 0;
}
//! [Finish]
