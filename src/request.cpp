/*!
 * @file       request.cpp
 * @brief      Defines the Request class
 * @author     Eddie Carle &lt;eddie@isatec.ca&gt;
 * @date       May 20, 2017
 * @copyright  Copyright &copy; 2017 Eddie Carle. This project is released under
 *             the GNU Lesser General Public License Version 3.
 */

/*******************************************************************************
* Copyright (C) 2017 Eddie Carle [eddie@isatec.ca]                             *
*                                                                              *
* This file is part of fastcgi++.                                              *
*                                                                              *
* fastcgi++ is free software: you can redistribute it and/or modify it under   *
* the terms of the GNU Lesser General Public License as  published by the Free *
* Software Foundation, either version 3 of the License, or (at your option)    *
* any later version.                                                           *
*                                                                              *
* fastcgi++ is distributed in the hope that it will be useful, but WITHOUT ANY *
* WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS    *
* FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for     *
* more details.                                                                *
*                                                                              *
* You should have received a copy of the GNU Lesser General Public License     *
* along with fastcgi++.  If not, see <http://www.gnu.org/licenses/>.           *
*******************************************************************************/

#include "fastcgi++/request.hpp"
#include "fastcgi++/log.hpp"

template<class charT> void Fastcgipp::Request<charT>::complete()
{
    vlog("%s kill %d\n", __PRETTY_FUNCTION__, m_kill);
    out.flush();
    err.flush();

    Block record(sizeof(Protocol::Header)+sizeof(Protocol::EndRequest));

    Protocol::Header& header
        = *reinterpret_cast<Protocol::Header*>(record.begin());
    header.version = Protocol::version;
    header.type = Protocol::RecordType::END_REQUEST;
    header.fcgiId = m_id.m_id;
    header.contentLength = sizeof(Protocol::EndRequest);
    header.paddingLength = 0;

    Protocol::EndRequest& body =
        *reinterpret_cast<Protocol::EndRequest*>(record.begin()+sizeof(header));
    body.appStatus = 0;
    body.protocolStatus = m_status;

    m_send(m_id.m_socket, std::move(record), m_kill);
}

template<class charT>
std::unique_lock<std::mutex>Fastcgipp::Request<charT>::handler()
{
    std::unique_lock<std::mutex> lock(m_messagesMutex);
    while(!needUpgrade() && !m_messages.empty())
    {
        vlog("in while %s needUpgrade %d m_messages.size() %d\n", __func__, needUpgrade(), m_messages.size());
        Message message = std::move(m_messages.front());
        m_messages.pop();
        lock.unlock();

        if(message.type == 0)
        {
            const Protocol::Header& header =
                *reinterpret_cast<Protocol::Header*>(message.data.begin());
            const auto body = message.data.begin()+sizeof(header);
            const auto bodyEnd = body+header.contentLength;

            if(header.type == Protocol::RecordType::ABORT_REQUEST)
            {
                complete();
                goto exit;
            }

            if(header.type != m_state)
            {
                WARNING_LOG("Records received out of order from web server")
                errorHandler();
                complete();
                goto exit;
            }

            switch(m_state)
            {
                case Protocol::RecordType::PARAMS:
                {
                    if(!(
                                role()==Protocol::Role::RESPONDER
                                || role()==Protocol::Role::AUTHORIZER))
                    {
                        m_status = Protocol::ProtocolStatus::UNKNOWN_ROLE;
                        WARNING_LOG("We got asked to do an unknown role")
                        errorHandler();
                        complete();
                        goto exit;
                    }

                    if(header.contentLength == 0)
                    {
                        if(environment().contentLength > m_maxPostSize)
                        {
                            bigPostErrorHandler();
                            complete();
                            goto exit;
                        }
                        m_state = Protocol::RecordType::IN;
                        needUpgrade(true);
                        vwlog(L"request parsed requestUri %ls\n", environment().requestUri.c_str());
                        vlog("needUpgrade(true); m_messages.size %zd\n", m_messages.size());
                        lock.lock();
                        continue;
                    }
                    m_environment.fill(body,  bodyEnd);
                    lock.lock();
                    continue;
                }

                case Protocol::RecordType::IN:
                {
                    if(header.contentLength==0)
                    {
                        if(!inProcessor() && !m_environment.parsePostBuffer())
                        {
                            WARNING_LOG("Unknown content type from client")
                            unknownContentErrorHandler();
                            complete();
                            goto exit;
                        }

                        m_environment.clearPostBuffer();
                        m_state = Protocol::RecordType::OUT;
                        break;
                    }

                    if(m_environment.postBuffer().size()+(bodyEnd-body)
                            > environment().contentLength)
                    {
                        bigPostErrorHandler();
                        complete();
                        goto exit;
                    }

                    m_environment.fillPostBuffer(body, bodyEnd);
                    inHandler(header.contentLength);
                    lock.lock();
                    continue;
                }

                default:
                {
                    ERROR_LOG("Our request is in a weird state.")
                    errorHandler();
                    complete();
                    goto exit;
                }
            }
        }

        m_message = std::move(message);
        if(response())
        {
            complete();
            break;
        }
        lock.lock();
    }
exit:
    vlog("exit %s needUpgrade %d m_messages.size() %d\n", __func__, needUpgrade(), m_messages.size());
    return lock;
}

template<class charT> void Fastcgipp::Request<charT>::errorHandler()
{
    out << \
"Status: 500 Internal Server Error\n"\
"Content-Type: text/html; charset=utf-8\r\n\r\n"\
"<!DOCTYPE html>"\
"<html lang='en'>"\
    "<head>"\
        "<title>500 Internal Server Error</title>"\
    "</head>"\
    "<body>"\
        "<h1>500 Internal Server Error</h1>"\
    "</body>"\
"</html>";
}

template<class charT> void Fastcgipp::Request<charT>::bigPostErrorHandler()
{
        out << \
"Status: 413 Request Entity Too Large\n"\
"Content-Type: text/html; charset=utf-8\r\n\r\n"\
"<!DOCTYPE html>"\
"<html lang='en'>"\
    "<head>"\
        "<title>413 Request Entity Too Large</title>"\
    "</head>"\
    "<body>"\
        "<h1>413 Request Entity Too Large</h1>"\
    "</body>"\
"</html>";
}

template<class charT> void Fastcgipp::Request<charT>::unknownContentErrorHandler()
{
        out << \
"Status: 415 Unsupported Media Type\n"\
"Content-Type: text/html; charset=utf-8\r\n\r\n"\
"<!DOCTYPE html>"\
"<html lang='en'>"\
    "<head>"\
        "<title>415 Unsupported Media Type</title>"\
    "</head>"\
    "<body>"\
        "<h1>415 Unsupported Media Type</h1>"\
    "</body>"\
"</html>";
}

template<class charT> bool Fastcgipp::Request<charT>::response() {
    vlog("\n*** response not implemented. return 404.\n\n");
    out << 
R"(Status: 404 Not Found
Content-Type: text/html; charset=utf-8

<!DOCTYPE html>
<html lang='en'>
    <head>
        <title>Not Implemented</title>
    </head>
    <body>
        <h1>Not Implemented</h1>
        <h2>This is the default reponse()</h2>
    </body>
</html>)";
    return true;
}

template<class charT> void Fastcgipp::Request<charT>::configure(
        const Protocol::RequestId& id,
        const Protocol::Role& role,
        bool kill,
        const std::function<void(const Socket&, Block&&, bool)> send,
        const std::function<void(Message)> callback)
{
    using namespace std::placeholders;

    m_kill=kill;
    m_id=id;
    m_role=role;
    m_callback=callback;
    m_send=send;

    m_outStreamBuffer.configure(
            id,
            Protocol::RecordType::OUT,
            std::bind(send, _1, _2, false));
    m_errStreamBuffer.configure(
            id,
            Protocol::RecordType::ERR,
            std::bind(send, _1, _2, false));
}

template<class charT> void Fastcgipp::Request<charT>::configureOp(
        const std::function<void(const Socket&, Block&&, bool)> send,
        const std::function<void(Message)> callback)
{
    using namespace std::placeholders;
    
    m_callback=callback;
    m_send=send;

    m_outStreamBuffer.configure(
            m_id,
            Protocol::RecordType::OUT,
            std::bind(send, _1, _2, false));
    m_errStreamBuffer.configure(
            m_id,
            Protocol::RecordType::ERR,
            std::bind(send, _1, _2, false));
}

template<class charT> unsigned Fastcgipp::Request<charT>::pickLocale(
        const std::vector<std::string>& locales)
{
    unsigned index=0;

    for(const std::string& language: environment().acceptLanguages)
    {
        if(language.size() <= 5)
        {
            const auto it = std::find_if(
                    locales.cbegin(),
                    locales.cend(),
                    [&language] (const std::string& locale)
                    {
                        return std::equal(
                                language.cbegin(),
                                language.cend(),
                                locale.cbegin());
                    });

            if(it != locales.cend())
            {
                index = it-locales.cbegin();
                break;
            }
        }
    }

    return index;
}

template<class charT> void Fastcgipp::Request<charT>::setLocale(
        const std::string& locale)
{
    try
    {
        out.imbue(std::locale(locale+codepage()));
    }
    catch(...)
    {
        ERROR_LOG("Unable to set locale")
        out.imbue(std::locale("C"));
    }
}

namespace Fastcgipp
{
    template<> const char* Fastcgipp::Request<wchar_t>::codepage() const
    {
        return ".UTF-8";
    }

    template<> const char* Fastcgipp::Request<char>::codepage() const
    {
        return "";
    }
}

template class Fastcgipp::Request<char>;
template class Fastcgipp::Request<wchar_t>;
