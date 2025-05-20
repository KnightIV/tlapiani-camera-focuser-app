#include "widget.h"

#include <QLabel>
#include <QPushButton>
#include <QGridLayout>
#include <QString>

#include <QtLogging>
#include <QTimer> // only used for mock client

#include "focuscontroller.h"

class MockPwi4Client : public IPWI4Client {

private:
    bool m_isEnabled = false;
    bool m_isConnected = false;
    bool m_isMoving = false;

    float m_position = 0;

    // IPWI4Client interface
public:

    // MockPwi4Client() : IPWI4Client() {}

    virtual void focuserConnect() override {
        m_isConnected = true;
    }

    virtual void focuserDisconnect() override {
        m_isConnected = false;
    }

    virtual void focuserEnable() override {
        m_isEnabled = true;
    }
    virtual void focuserDisable() override {
        m_isEnabled = false;
    }

    virtual void focuserGoto(float target) override {
        this->m_position = target;
        this->m_isMoving = true;

        QTimer::singleShot(500, [=]() {
            this->m_isMoving = false;
        });
    }

    virtual void focuserStop() override {
        qInfo() << "Called stop";
        this->m_isMoving = false;
    }

    virtual bool focuserIsConnected() override {
        return m_isConnected;
    }

    virtual bool focuserIsEnabled() override {
        return m_isEnabled;
    }

    virtual bool focuserIsMoving() override {
        return m_isMoving;
    }

    virtual float focuserPosition() override {
        return m_position;
    }
};

Widget::Widget(QWidget *parent) : QWidget(parent) {
    setWindowTitle("Tlapiani - Camara y Enfocador");
    resize(1000, 800);

    QGridLayout *layout = new QGridLayout(this);
    for (int r = 0; r < 2; r++) {
        for (int c = 0; c < 2; c++) {
            FocusController *controller = new FocusController(this, new MockPwi4Client());
            layout->addWidget(controller, r, c);
        }
    }
}

Widget::~Widget() {

}
