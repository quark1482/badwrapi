#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include <QApplication>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

#define HTTP_HEADER_CONTENT_TYPE_HTML "text/html"

#define HTTP_HEADER_CONTENT_TYPE_JSON "application/json"

#define HTTP_HEADER_CONTENT_TYPE_FORM "application/x-www-form-urlencoded"

typedef enum {
    HTTP_METHOD_HEAD,
    HTTP_METHOD_GET,
    HTTP_METHOD_POST
} HTTPMethod;

typedef enum {
    HTTP_STATUS_INVALID=0,
    HTTP_STATUS_CONTINUE=100,
    HTTP_STATUS_OK=200,
    HTTP_STATUS_CREATED=201,
    HTTP_STATUS_ACCEPTED=202,
    HTTP_STATUS_NO_CONTENT=204,
    HTTP_STATUS_RESET_CONTENT=205,
    HTTP_STATUS_PARTIAL_CONTENT=206,
    HTTP_STATUS_MULTIPLE_CHOICES=300,
    HTTP_STATUS_MOVED_PERMANENTLY=301,
    HTTP_STATUS_FOUND=302,
    HTTP_STATUS_SEE_OTHER=303,
    HTTP_STATUS_NOT_MODIFIED=304,
    HTTP_STATUS_BAD_REQUEST=400,
    HTTP_STATUS_UNAUTHORIZED=401,
    HTTP_STATUS_FORBIDDEN=403,
    HTTP_STATUS_NOT_FOUND=404,
    HTTP_STATUS_INTERNAL_SERVER_ERROR=500,
    HTTP_STATUS_NOT_IMPLEMENTED=501,
    HTTP_STATUS_BAD_GATEWAY=502,
    HTTP_STATUS_SERVICE_UNAVAILABLE=503
} HTTPStatus;

typedef QHash<QByteArray,QByteArray> RawHeadersHash;

typedef QList<QPair<QNetworkRequest::Attribute,QVariant>> AttributesList;

class HTTPRequest {
public:
    static void get(QUrl,RawHeadersHash,AttributesList,uint &,QByteArray &,RawHeadersHash &,QString &);
    static void head(QUrl,RawHeadersHash,AttributesList,uint &,RawHeadersHash &,QString &);
    static void post(QUrl,QByteArray,RawHeadersHash,AttributesList,uint &,QByteArray &,RawHeadersHash &,QString &);
private:
    static void rawMethod(HTTPMethod,QUrl,QByteArray,RawHeadersHash,AttributesList,uint &,QByteArray &,RawHeadersHash &,QString &);
};

#endif // HTTPREQUEST_H
