#pragma once

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QString>
#include <QLineEdit>

#include "ipwi4client.h"

class FocusController : public QWidget {
    Q_OBJECT

private:
    int m_focuserId;
    float m_stepSize;
    IPWI4Client *m_pwi;

    QPushButton *m_connectBtn, *m_disconnectBtn;
    QPushButton *m_enableBtn, *m_disableBtn;

    QLabel *m_positionLabel;
    QLineEdit *m_stepSizeText;
    QPushButton *m_upStepBtn, *m_downStepBtn;

    QLabel *m_isMovingLabel;
    QPushButton *m_stopBtn;

    static int getNextId();

    void setupWidget();
    void checkPwiConnection();

public:
    explicit FocusController(QWidget *parent, IPWI4Client *pwi);

    ~FocusController();

public slots:
    void connectPwi(bool isChecked);

    void updatePosition();
    void updateStepSize(const QString &textChanged);
    void updateIsMoving();

signals:
};
