#pragma once

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <chrono>
#include <QStackedLayout>

#include "ipwi4client.h"

class FocusController : public QWidget {
    Q_OBJECT

private:
    static constexpr std::chrono::duration<double, std::milli> CONNECT_TIMEOUT = std::chrono::milliseconds(500);

    int m_focuserId;
    float m_stepSize;
    bool m_emergencyStopped;
    IPWI4Client *m_pwi;

    std::chrono::duration<double, std::milli> m_connectRequestTime;

    QStackedLayout *m_mainLayout;

    QPushButton *m_connectBtn, *m_disconnectBtn;

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
    void checkPwiExists();
    void moveFocuser(float newPosition);

public:
    explicit FocusController(QWidget *parent, IPWI4Client *pwi);

    ~FocusController();

private slots:
    void doPwiConnect(bool isConnecting);

    void updatePosition();
    void updateStepSize(const QString &textChanged);
    void updateIsMoving();

signals:
};
