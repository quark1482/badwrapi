#ifndef TAGEOCODER_H
#define TAGEOCODER_H

#include <QtCore>
#include "httprequest.h"

#define URL_TRIPADVISOR     "https://www.tripadvisor.com"

#define PATH_QUERY_LOCATION "/data/graphql/ids"

class TAGeoCoder {
public:
    static bool getLatLng(QString,float &,float &,QString &);
    static void setAgent(QString);
    static void setProxy(QNetworkProxy *);
private:
    static bool findQueryId(QString &,QStringList &,QString &);
    static bool getQueryIdFromScript(QString,QString &,QString &);
    static inline QString       sAPIUserAgent=QStringLiteral("Mozilla/5.0");
    static inline QNetworkProxy *pxyAPIProxy=nullptr;
};

#endif // TAGEOCODER_H
