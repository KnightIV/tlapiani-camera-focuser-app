#include "widget.h"

#include <QLabel>
#include <QPushButton>
#include <QGridLayout>
#include <QString>

#include <QtLogging>

#include "focuscontroller.h"

class MockPwi4Client : public IPWI4Client{

private:
    bool m_isEnabled;
    bool m_isConnected;
    bool m_isMoving;

    float m_position;

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
    }

    virtual void focuserStop() override {
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
    resize(500, 400);

    // QLabel *label = new QLabel(this);
    // label->setText("label 1");

    // QLabel *label2 = new QLabel(this);
    // label2->setText("label 2");

    // UANL logo example
    // label->setGeometry(20, 20, 800, 800);
    // label->setPixmap(QPixmap(":/images/UANL Logo.png"));

    QGridLayout *layout = new QGridLayout(this);
    // layout->addWidget(label, 0, 1);
    // layout->addWidget(label2, 1, 0);

    // qInfo() << layout->columnCount();

    // for (int i = 0; i < 2; i++) {
    //     for (int j = 0; j < 2; j++) {
    //         QLabel *label = new QLabel(this);
    //         label->setText(QString::number((i*2) + j));
    //         layout->addWidget(label, i, j);
    //     }
    // }

    FocusController *c = new FocusController(this, new MockPwi4Client());
    // c->setMinimumHeight(10);
    // layout->addWidget(c, 0, 0, Qt::AlignHCenter);
    layout->addWidget(c, 0, 0);
}

Widget::~Widget() {

}
