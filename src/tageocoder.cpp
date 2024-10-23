#include "tageocoder.h"

#define REGEX_FIND_SCRIPT     R"(\s*<\s*(https?:\/\/.+\.js)\s*>\s*.*\s*;\s*rel\s*=\s*\"preload\")"

#define REGEX_FIND_QUERY_ID   R"(startQuery\s*:\s*[a-zA-Z_]+\s*,\s*typeaheadId\s*:\s*[a-zA-Z_]+\s*}\s*}\s*,\s*[a-zA-Z_]+\s*=\s*{\s*__key\s*:\s*[0-9a-fx]+\s*,\s*id\s*:\s*\"([0-9a-fx]+)\"\s*,\s*loc\s*:\s*{}\s*,\s*definitions\s*:\s*\[\]\s*})"

#define DEFAULT_ERROR_MESSAGE "Unexpected response"

#define PAYLOAD_GEOCODING     R"(
[
    {
        "variables": {
            "request": {
                "query": "",
                "limit": 1,
                "scope": "WORLDWIDE",
                "locale": "en-US",
                "scopeGeoId": 1,
                "searchCenter": null,
                "types": [
                    "LOCATION"
                ],
                "locationTypes": [
                    "GEO"
                ],
                "userId": null,
                "includeRecent": true
            }
        },
        "extensions": {
            "preRegisteredQueryId": ""
        }
    }
]
)"

bool TAGeoCoder::getLatLng(QString sLocation,
                           float   &fLatitude,
                           float   &fLongitude,
                           QString &sError) {
    bool           bResult=false;
    uint           uiResCode;
    QString        sQueryId,
                   sRequest,
                   sContentType;
    QStringList    slCookies;
    QByteArray     abtBody;
    QJsonDocument  jsnDoc;
    QJsonObject    jsnObj,
                   jsnVar,
                   jsnReq,
                   jsnExt;
    QJsonArray     jsnArr;
    RawHeadersHash rhhReqHeaders,
                   rhhReSHeaders;
    QUrl           urlEndPoint;
    fLatitude=qQNaN();
    fLongitude=qQNaN();
    sError.clear();
    if(findQueryId(sQueryId,slCookies,sError)) {
        jsnDoc=QJsonDocument::fromJson(QStringLiteral(PAYLOAD_GEOCODING).toUtf8());
        jsnArr=jsnDoc.array();
        jsnObj=jsnArr[0].toObject();
        jsnVar=jsnObj.value(QStringLiteral("variables")).toObject();
        jsnReq=jsnVar.value(QStringLiteral("request")).toObject();
        jsnExt=jsnObj.value(QStringLiteral("extensions")).toObject();
        jsnReq[QStringLiteral("query")]=sLocation;
        jsnExt[QStringLiteral("preRegisteredQueryId")]=sQueryId;
        jsnVar[QStringLiteral("request")]=jsnReq;
        jsnObj[QStringLiteral("variables")]=jsnVar;
        jsnObj[QStringLiteral("extensions")]=jsnExt;
        jsnArr[0]=jsnObj;
        jsnDoc.setArray(jsnArr);
        sRequest=jsnDoc.toJson(QJsonDocument::JsonFormat::Compact);
        rhhReqHeaders.clear();
        rhhReqHeaders[QStringLiteral("user-agent").toUtf8()]=sAPIUserAgent.toUtf8();
        rhhReqHeaders[QStringLiteral("content-type").toUtf8()]=QStringLiteral(HTTP_HEADER_CONTENT_TYPE_JSON).toUtf8();
        for(const auto &c:slCookies)
            if(rhhReqHeaders.contains(QStringLiteral("cookie").toUtf8()))
                rhhReqHeaders[QStringLiteral("cookie").toUtf8()].append(
                    QStringLiteral("; %1").arg(c).toUtf8()
                );
            else
                rhhReqHeaders.insert(QStringLiteral("cookie").toUtf8(),c.toUtf8());
        urlEndPoint=QUrl(
            QStringLiteral("%1%2").arg(
                QStringLiteral(URL_TRIPADVISOR),
                QStringLiteral(PATH_QUERY_LOCATION)
            )
        );
        HTTPRequest::post(
            urlEndPoint,
            sRequest.toUtf8(),
            rhhReqHeaders,
            {},
            pxyAPIProxy,
            uiResCode,
            abtBody,
            rhhReSHeaders,
            sError
        );
        sContentType=rhhReSHeaders.value(QStringLiteral("content-type").toUtf8());
        if(HTTP_STATUS_OK==uiResCode)
            if(sContentType.startsWith(QStringLiteral(HTTP_HEADER_CONTENT_TYPE_JSON))) {
                jsnDoc=QJsonDocument::fromJson(abtBody);
                if(!jsnDoc.isNull()) {
                    jsnObj=jsnDoc.object();
                    if(jsnDoc.isArray())
                        if(jsnDoc.array().count())
                            jsnObj=jsnDoc.array()[0].toObject();
                    if(jsnObj.contains(QStringLiteral("data")))
                        if(jsnObj.value(QStringLiteral("data")).isObject())
                            jsnObj=jsnObj.value(QStringLiteral("data")).toObject();
                    if(jsnObj.contains(QStringLiteral("Typeahead_autocomplete")))
                        if(jsnObj.value(QStringLiteral("Typeahead_autocomplete")).isObject())
                            jsnObj=jsnObj.value(QStringLiteral("Typeahead_autocomplete")).toObject();
                    if(jsnObj.contains(QStringLiteral("results")))
                        if(jsnObj.value(QStringLiteral("results")).isArray()) {
                            QJsonArray jsnRes;
                            jsnRes=jsnObj.value(QStringLiteral("results")).toArray();
                            if(!jsnRes.count())
                                sError=QStringLiteral("Couldn't geocode the supplied location");
                            else
                                if(jsnRes[0].isObject())
                                    jsnObj=jsnRes[0].toObject();
                        }
                    if(jsnObj.contains(QStringLiteral("details")))
                        if(jsnObj.value(QStringLiteral("details")).isObject()) {
                            jsnObj=jsnObj.value(QStringLiteral("details")).toObject();
                            if(jsnObj.contains(QStringLiteral("latitude"))&&
                               jsnObj.contains(QStringLiteral("longitude"))
                            )
                                if(jsnObj.value(QStringLiteral("latitude")).isDouble()&&
                                   jsnObj.value(QStringLiteral("longitude")).isDouble()) {
                                    fLatitude=jsnObj.value(QStringLiteral("latitude")).toDouble();
                                    fLongitude=jsnObj.value(QStringLiteral("longitude")).toDouble();
                                    bResult=true;
                                }
                        }
                    if(!bResult)
                        if(sError.isEmpty())
                            sError=QStringLiteral("The geocode request failed");
                }
                else
                    sError=QStringLiteral("Unexpected content");
            }
            else
                sError=QStringLiteral("Unexpected content type: %1").
                       arg(sContentType);
        else if(HTTP_STATUS_INVALID!=uiResCode)
            sError=QStringLiteral("Unexpected response code: %1").
                   arg(uiResCode);
    }
    if(!bResult) {
        if(sError.isEmpty())
            sError=QStringLiteral(DEFAULT_ERROR_MESSAGE);
        sError=QStringLiteral("[%1()] %2").arg(__FUNCTION__,sError);
    }
    return bResult;
}

void TAGeoCoder::setAgent(QString sAgent) {
    sAPIUserAgent=sAgent;
}

void TAGeoCoder::setProxy(QNetworkProxy *pxyProxy) {
    pxyAPIProxy=pxyProxy;
}

bool TAGeoCoder::findQueryId(QString     &sQueryId,
                             QStringList &slCookies,
                             QString     &sError) {
    bool           bResult=false;
    uint           uiResCode;
    QString        sContentType,
                   sCookies;
    RawHeadersHash rhhHeaders;
    sQueryId.clear();
    slCookies.clear();
    sError.clear();
    HTTPRequest::head(
        QUrl(QStringLiteral(URL_TRIPADVISOR)),
        {
            {
                QStringLiteral("user-agent").toUtf8(),
                sAPIUserAgent.toUtf8()
            }
        },
        {},
        pxyAPIProxy,
        uiResCode,
        rhhHeaders,
        sError
    );
    sContentType=rhhHeaders.value(QStringLiteral("content-type").toUtf8());
    sCookies=rhhHeaders.value(QStringLiteral("set-cookie").toUtf8());
    slCookies=sCookies.split(QStringLiteral("\n"));
    for(auto &c:slCookies)
        c=c.split(QStringLiteral("; "))[0];
    if(HTTP_STATUS_OK==uiResCode)
        if(sContentType.startsWith(QStringLiteral(HTTP_HEADER_CONTENT_TYPE_HTML)))
            if(rhhHeaders.contains(QStringLiteral("link").toUtf8())) {
                QString            sLinks=rhhHeaders.value(QStringLiteral("link").toUtf8());
                QStringList        slLinks=sLinks.split(QStringLiteral("\n")),
                                   slScripts;
                QRegularExpression rxRegEx;
                rxRegEx.setPattern(QStringLiteral(REGEX_FIND_SCRIPT));
                slScripts.clear();
                for(const auto &l:slLinks)
                    for(const auto &m:l.split(QStringLiteral(", "))) {
                        QRegularExpressionMatch rxmMatch=rxRegEx.match(m);
                        if(rxmMatch.hasMatch())
                            slScripts.append(rxmMatch.captured(1));
                    }
                if(slScripts.length()) {
                    for(const auto &s:slScripts)
                        if(getQueryIdFromScript(s,sQueryId,sError))
                            break;
                    if(sQueryId.isEmpty())
                        sError=QStringLiteral("Unable to find the query id in any script");
                    else
                        bResult=true;
                }
                else
                    sError=QStringLiteral("No 'preload' scripts were found in the headers");
            }
            else
                sError=QStringLiteral("No 'Link' headers found");
        else
            sError=QStringLiteral("Unexpected content type: %1").
                   arg(sContentType);
    else if(HTTP_STATUS_INVALID!=uiResCode)
        sError=QStringLiteral("Unexpected response code: %1").
               arg(uiResCode);
    if(!bResult) {
        if(sError.isEmpty())
            sError=QStringLiteral(DEFAULT_ERROR_MESSAGE);
        sError=QStringLiteral("[%1()] %2").arg(__FUNCTION__,sError);
    }
    return bResult;
}


bool TAGeoCoder::getQueryIdFromScript(QString sURL,
                                      QString &sQueryId,
                                      QString &sError) {
    bool           bResult=false;
    uint           uiResCode;
    QString        sContentType;
    QByteArray     abtBody;
    RawHeadersHash rhhHeaders;
    sQueryId.clear();
    sError.clear();
    HTTPRequest::get(
        QUrl(sURL),
        {
            {
                QStringLiteral("user-agent").toUtf8(),
                sAPIUserAgent.toUtf8()
            }
        },
        {},
        pxyAPIProxy,
        uiResCode,
        abtBody,
        rhhHeaders,
        sError
    );
    sContentType=rhhHeaders.value(QStringLiteral("content-type").toUtf8());
    if(HTTP_STATUS_OK==uiResCode)
        if(sContentType.contains(QStringLiteral("javascript"))) {
            QRegularExpression      rxRegEx;
            QRegularExpressionMatch rxmMatch;
            rxRegEx.setPattern(QStringLiteral(REGEX_FIND_QUERY_ID));
            rxmMatch=rxRegEx.match(abtBody);
            if(rxmMatch.hasMatch()) {
                sQueryId=rxmMatch.captured(1);
                bResult=true;
            }
            else
                sError=QStringLiteral("Unable to find the query id in the script source");
        }
        else
            sError=QStringLiteral("Unexpected content type: %1").
                   arg(sContentType);
    else if(HTTP_STATUS_INVALID!=uiResCode)
        sError=QStringLiteral("Unexpected response code: %1").
               arg(uiResCode);
    if(!bResult) {
        if(sError.isEmpty())
            sError=QStringLiteral(DEFAULT_ERROR_MESSAGE);
        sError=QStringLiteral("[%1()] %2").arg(__FUNCTION__,sError);
    }
    return bResult;
}
