#pragma once

#include <QWidget>
#include <QPushButton>
#include <QLabel>
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
    QLineEdit *m_stepSizeText, *m_goPositionText;
    QPushButton *m_upStepBtn, *m_downStepBtn, *m_goPositionBtn;

    QLabel *m_isMovingLabel;
    QPushButton *m_stopBtn;

    static int getNextId();

    void setupWidget();
    void setupControls();

    void setEnabledControls(bool enable);
    void checkPwiConnection();
    void checkPwiEnabled();
    void moveFocuser(float newPosition);

public:
    explicit FocusController(QWidget *parent, IPWI4Client *pwi);

    ~FocusController();

private slots:
    void doPwiConnect(bool isConnecting);
    void doPwiEnable(bool isEnabling);

    void updatePosition();
    void updateStepSize(const QString &textChanged);
    void updateIsMoving();

signals:
};
