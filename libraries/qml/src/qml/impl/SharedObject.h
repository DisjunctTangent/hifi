//
//  Created by Bradley Austin Davis on 2018-01-04
//  Copyright 2013-2018 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#pragma once

#include <QtCore/QObject>
#include <QtCore/QSharedPointer>
#include <QtCore/QThread>
#include <QtCore/QWaitCondition>
#include <QtCore/QMutex>
#include <QtCore/QSize>

#include "TextureCache.h"


class QWindow;
class QTimer;
class QQuickWindow;
class QQuickItem;
class QOpenGLContext;
class QQmlEngine;
class QQmlContext;
class OffscreenGLCanvas;

namespace hifi { namespace qml {

class OffscreenSurface;

namespace impl {

class RenderControl;
class RenderEventHandler;

class SharedObject : public QObject {
    Q_OBJECT

    friend class RenderEventHandler;

public:
    static void setSharedContext(QOpenGLContext* context);
    static QOpenGLContext* getSharedContext();
    static TextureCache& getTextureCache();

    SharedObject();
    virtual ~SharedObject();

    void create(OffscreenSurface* surface);
    void setRootItem(QQuickItem* rootItem);
    void destroy();
    bool isQuit();

    QSize getSize() const;
    void setSize(const QSize& size);
    void setMaxFps(uint8_t maxFps) { _maxFps = maxFps; }

    QQuickWindow* getWindow() { return _quickWindow; }
    QQuickItem* getRootItem() { return _rootItem; }
    QQmlContext* getContext() { return _qmlContext; }
    void setProxyWindow(QWindow* window);

    void pause();
    void resume();
    bool isPaused() const;
    bool fetchTexture(TextureAndFence& textureAndFence);


private:
    bool event(QEvent* e) override;

    bool preRender();
    void shutdownRendering(OffscreenGLCanvas& canvas, const QSize& size);
    // Called by the render event handler, from the render thread
    void initializeRenderControl(QOpenGLContext* context);
    void releaseTextureAndFence();
    void setRenderTarget(uint32_t fbo, const QSize& size);

    QQmlEngine* acquireEngine(OffscreenSurface* surface);
    void releaseEngine(QQmlEngine* engine);

    void requestRender();
    void requestRenderSync();
    void wait();
    void wake();
    void onInitialize();
    void onRender();
    void onTimer();
    void onAboutToQuit();
    void updateTextureAndFence(const TextureAndFence& newTextureAndFence);

    // Texture management
    TextureAndFence _latestTextureAndFence{ 0, 0 };
    RenderControl* _renderControl{ nullptr };
    RenderEventHandler* _renderObject{ nullptr };
    QQuickWindow* _quickWindow{ nullptr };
    QWindow* _proxyWindow{ nullptr };
    QQuickItem* _item{ nullptr };
    QQuickItem* _rootItem{ nullptr };
    QQmlContext* _qmlContext{ nullptr };
    QTimer* _renderTimer{ nullptr };
    QThread* _renderThread{ nullptr };
    QWaitCondition _cond;
    mutable QMutex _mutex;

    uint64_t _lastRenderTime{ 0 };
    QSize _size{ 100, 100 };
    uint8_t _maxFps{ 60 };

    bool _renderRequested{ false };
    bool _syncRequested{ false };
    bool _quit{ false };
    bool _paused{ false };
};

}  // namespace impl
}}  // namespace hifi::qml
