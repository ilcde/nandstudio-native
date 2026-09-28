// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <QQuickWindow>
#include <atomic>
#include <memory>

// Development evidence only. Observes naturally produced frames; never requests
// a repaint, changes rendering, or reads GUI objects from the render thread.
class PresentationProbe {
    struct State {
        std::atomic<quint64> requested{1}, synchronized{0}, submitted{0};
    };
    std::shared_ptr<State> state_=std::make_shared<State>();
public:
    explicit PresentationProbe(QQuickWindow* window) {
        QObject::connect(window,&QQuickWindow::afterSynchronizing,window,[s=state_]{
            s->synchronized.store(s->requested.load());
        },Qt::DirectConnection);
        QObject::connect(window,&QQuickWindow::frameSwapped,window,[s=state_]{
            s->submitted.store(s->synchronized.load());
        },Qt::DirectConnection);
    }
    void changed(){state_->requested.fetch_add(1);}
    quint64 requested()const{return state_->requested.load();}
    quint64 submitted()const{return state_->submitted.load();}
};
