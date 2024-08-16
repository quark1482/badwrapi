#include "httprequest.h"

void HTTPRequest::get(QUrl            urlURL,
                      RawHeadersHash  rhhRequestHeaders,
                      AttributesList  alAttributes,
                      QNetworkProxy   *pxyProxy,
                      uint            &uiResponseCode,
                      QByteArray      &abtResponseContents,
                      RawHeadersHash  &rhhResponseHeaders,
                      QString         &sError) {
    rawMethod(
        HTTP_METHOD_GET,
        urlURL,
        QByteArray(),
        rhhRequestHeaders,
        alAttributes,
        pxyProxy,
        uiResponseCode,
        abtResponseContents,
        rhhResponseHeaders,
        sError
    );
    if(!sError.isEmpty())
        sError=QStringLiteral("[%1()] %2").arg(__FUNCTION__,sError);
}

void HTTPRequest::head(QUrl            urlURL,
                       RawHeadersHash  rhhRequestHeaders,
                       AttributesList  alAttributes,
                       QNetworkProxy   *pxyProxy,
                       uint            &uiResponseCode,
                       RawHeadersHash  &rhhResponseHeaders,
                       QString         &sError) {
    QByteArray abtDummy;
    rawMethod(
        HTTP_METHOD_HEAD,
        urlURL,
        QByteArray(),
        rhhRequestHeaders,
        alAttributes,
        pxyProxy,
        uiResponseCode,
        abtDummy,
        rhhResponseHeaders,
        sError
    );
    if(!sError.isEmpty())
        sError=QStringLiteral("[%1()] %2").arg(__FUNCTION__,sError);
}

void HTTPRequest::post(QUrl            urlURL,
                       QByteArray      abtPayload,
                       RawHeadersHash  rhhRequestHeaders,
                       AttributesList  alAttributes,
                       QNetworkProxy   *pxyProxy,
                       uint            &uiResponseCode,
                       QByteArray      &abtResponseContents,
                       RawHeadersHash  &rhhResponseHeaders,
                       QString         &sError) {
    rawMethod(
        HTTP_METHOD_POST,
        urlURL,
        abtPayload,
        rhhRequestHeaders,
        alAttributes,
        pxyProxy,
        uiResponseCode,
        abtResponseContents,
        rhhResponseHeaders,
        sError
    );
    if(!sError.isEmpty())
        sError=QStringLiteral("[%1()] %2").arg(__FUNCTION__,sError);
}

void HTTPRequest::rawMethod(HTTPMethod      hmMethod,
                            QUrl            urlURL,
                            QByteArray      abtPayload,
                            RawHeadersHash  rhhRequestHeaders,
                            AttributesList  alAttributes,
                            QNetworkProxy   *pxyProxy,
                            uint            &uiResponseCode,
                            QByteArray      &abtResponseContents,
                            RawHeadersHash  &rhhResponseHeaders,
                            QString         &sError) {
    QList<QByteArray>     abtlHeaders;
    QNetworkAccessManager namManager;
    QNetworkRequest       nrqRequest;
    QNetworkReply         *nrpReply;
    uiResponseCode=HTTP_STATUS_INVALID;
    abtResponseContents.clear();
    rhhResponseHeaders.clear();
    sError.clear();
    if(pxyProxy!=nullptr)
        namManager.setProxy(*pxyProxy);
    namManager.setTransferTimeout();
    nrqRequest.setUrl(urlURL);
    abtlHeaders=rhhRequestHeaders.keys();
    for(const auto &h:abtlHeaders)
        nrqRequest.setRawHeader(h,rhhRequestHeaders.value(h));
    for(const auto &a:alAttributes)
        nrqRequest.setAttribute(a.first,a.second);
    switch(hmMethod) {
        case HTTP_METHOD_HEAD:
            nrpReply=namManager.head(nrqRequest);
            break;
        case HTTP_METHOD_GET:
            nrpReply=namManager.get(nrqRequest);
            break;
        case HTTP_METHOD_POST:
            nrpReply=namManager.post(nrqRequest,abtPayload);
            break;
        default:
            nrpReply=nullptr;
    }
    if(nullptr==nrpReply)
        sError=QStringLiteral("Unsupported method");
    else {
        while(!nrpReply->isFinished())
            QApplication::processEvents(QEventLoop::ProcessEventsFlag::ExcludeUserInputEvents);
        uiResponseCode=nrpReply->attribute(
            QNetworkRequest::Attribute::HttpStatusCodeAttribute
        ).toUInt();
        for(const auto &h:nrpReply->rawHeaderPairs())
            // The HTTP header fields are case insensitive. We use the lowercase version here.
            // Use a lowercase key for accessing the rhmResponseHeaders values in the caller.
            if(rhhResponseHeaders.contains(h.first.toLower()))
                // If the same field appears multiple times in the headers, their values ...
                // ... are unified in a single entry, separated by semicolon-and-space.
                // Usual examples: 'set-cookie' and 'content-security-policy'.
                // 2024-08-10: I am returning them separated by new-lines, because the ...
                // ... semicolon-and-space approach is also used in one-line headers.
                rhhResponseHeaders[h.first.toLower()].append(
                    QStringLiteral("\n%1").arg(h.second).toUtf8()
                );
            else
                rhhResponseHeaders.insert(h.first.toLower(),h.second);
        abtResponseContents=nrpReply->readAll();
        if(QNetworkReply::NetworkError::NoError!=nrpReply->error())
            sError=nrpReply->errorString();
        else if(HTTP_STATUS_INVALID==uiResponseCode)
            sError=QStringLiteral("Response timeout expired");
        nrpReply->~QNetworkReply();
    }
    if(!sError.isEmpty())
        sError=QStringLiteral("[%1()] %2").arg(__FUNCTION__,sError);
}
