#include "focuscontroller.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTimer>
#include <QLineEdit>
#include <QDoubleValidator>

#include <QDebug>

int FocusController::getNextId() {
    static int curId = 0;
    return ++curId;
}

void FocusController::setupWidget() {
    QWidget *container = new QWidget(this);
    QVBoxLayout *widgetLayout = new QVBoxLayout(container);

    // title
    QLabel *titleLabel = new QLabel();
    titleLabel->setText("Hedrick Focuser #" + QString::number(m_focuserId));
    titleLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    titleLabel->setStyleSheet("background-color: none; border: 1px inset black;");
    titleLabel->setMaximumHeight(48);
    titleLabel->setFont(QFont("Consolas", 32, QFont::Bold));
    widgetLayout->addWidget(titleLabel);

    // connect & enable
    QVBoxLayout *buttonLayout = new QVBoxLayout();

    QHBoxLayout *connectLayout = new QHBoxLayout();
    m_connectBtn = new QPushButton("Connect", this);
    m_disconnectBtn = new QPushButton("Disconnect", this);
    m_disconnectBtn->setDisabled(true);
    connectLayout->addWidget(m_connectBtn);
    connectLayout->addWidget(m_disconnectBtn);
    connect(m_connectBtn, &QPushButton::clicked, this, [=, this] {
        m_connectRequestTime = std::chrono::system_clock::now().time_since_epoch();
    });

    buttonLayout->addLayout(connectLayout);
    widgetLayout->addLayout(buttonLayout);

    // focuser position
    QHBoxLayout *positionLayout = new QHBoxLayout();

    QVBoxLayout *positionHalfLayout = new QVBoxLayout();

    QHBoxLayout *descriptionLayout = new QHBoxLayout();
    QLabel *positionDescriptionLabel = new QLabel("Position: ");
    positionDescriptionLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    positionDescriptionLabel->setFont(QFont("Consolas", 20));
    descriptionLayout->addWidget(positionDescriptionLabel, 2);

    m_positionLabel = new QLabel();
    m_positionLabel->setText("NaN");
    m_positionLabel->setFont(QFont("Consolas", 20));
    m_positionLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    descriptionLayout->addWidget(m_positionLabel, 2);

    QHBoxLayout *goPositionLayout = new QHBoxLayout();
    m_goPositionText = new QLineEdit();
    m_goPositionText->setValidator(new QDoubleValidator(1, 10'000, 1));
    goPositionLayout->addWidget(m_goPositionText);

    m_goPositionBtn = new QPushButton();
    m_goPositionBtn->setText("Go to position");
    goPositionLayout->addWidget(m_goPositionBtn, 5);

    positionHalfLayout->addLayout(descriptionLayout);
    positionHalfLayout->addLayout(goPositionLayout);
    positionLayout->addLayout(positionHalfLayout);

    QVBoxLayout *stepLayout = new QVBoxLayout();
    m_upStepBtn = new QPushButton();
    m_downStepBtn = new QPushButton();

    QPixmap arrowIcon(":images/Arrow Icon.png");
    m_upStepBtn->setIcon(QIcon(arrowIcon.transformed(QTransform().rotate(90))));
    m_downStepBtn->setIcon(QIcon(arrowIcon.transformed(QTransform().rotate(-90))));

    m_stepSizeText = new QLineEdit();
    m_stepSizeText->setValidator(new QDoubleValidator(1, 500, 1));
    m_stepSizeText->setText("10");
    m_stepSizeText->setAlignment(Qt::AlignHCenter);

    stepLayout->addWidget(m_upStepBtn);
    stepLayout->addWidget(m_stepSizeText);
    stepLayout->addWidget(m_downStepBtn);

    positionLayout->addLayout(stepLayout);
    widgetLayout->addLayout(positionLayout);

    // is-moving/stop
    QHBoxLayout *movingLayout = new QHBoxLayout();
    m_isMovingLabel = new QLabel();
    m_isMovingLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    m_isMovingLabel->setFont(QFont("Consolas", 20));

    m_stopBtn = new QPushButton();
    m_stopBtn->setStyleSheet("background-color: red; color: white;");
    m_stopBtn->setText("STOP");
    m_stopBtn->setFont(QFont("Consolas", 24));
    m_stopBtn->setDisabled(true);

    movingLayout->addWidget(m_isMovingLabel);
    movingLayout->addWidget(m_stopBtn);
    widgetLayout->addLayout(movingLayout);

    // event timer
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [=, this]() {
        bool isFocuserConnected = m_pwi->focuserIsConnected();
        setEnabledControls(isFocuserConnected);

        bool isConnecting = std::chrono::system_clock::now().time_since_epoch() - m_connectRequestTime < FocusController::CONNECT_TIMEOUT;

        m_connectBtn->setDisabled(isFocuserConnected || isConnecting);
        m_disconnectBtn->setEnabled(isFocuserConnected);
        m_stopBtn->setEnabled(isFocuserConnected);
    });
    connect(timer, &QTimer::timeout, this, &FocusController::updatePosition);
    connect(timer, &QTimer::timeout, this, &FocusController::updateIsMoving);
    connect(timer, &QTimer::timeout, this, &FocusController::checkPwiExists);
    connect(timer, &QTimer::timeout, this, [=, this]() {
        m_stepSize = m_stepSizeText->text().toFloat();
    });
    timer->start(100);

    this->setupControls();
    this->setEnabledControls(false);

    m_mainLayout->addWidget(container);
}

void FocusController::setupControls() {
    connect(m_connectBtn, &QPushButton::clicked, this, [=, this]() { doPwiConnect(true); });
    connect(m_disconnectBtn, &QPushButton::clicked, this, [=, this]() { doPwiConnect(false); });

    connect(m_stepSizeText, &QLineEdit::textChanged, this, &FocusController::updateStepSize);
    connect(m_upStepBtn, &QPushButton::clicked, this, [=, this]() {
        this->moveFocuser(m_pwi->focuserPosition() + m_stepSize);
        this->setEnabledControls(false);
    });
    connect(m_downStepBtn, &QPushButton::clicked, this, [=, this]() {
        this->moveFocuser(m_pwi->focuserPosition() - m_stepSize);
        this->setEnabledControls(false);
    });

    connect(m_goPositionBtn, &QPushButton::clicked, this, [=, this]() {
        float newPosition = m_goPositionText->text().toFloat();
        this->moveFocuser(newPosition);
    });

    connect(m_stopBtn, &QPushButton::clicked, this, [=, this]() {
        m_pwi->focuserStop();
        m_emergencyStopped = true;
    });
}

void FocusController::setEnabledControls(bool enable) {
    m_goPositionText->setEnabled(enable);
    m_goPositionBtn->setEnabled(enable);
    m_upStepBtn->setEnabled(enable);
    m_downStepBtn->setEnabled(enable);
    m_stepSizeText->setEnabled(enable);
}

void FocusController::checkPwiConnection() {
    bool isConnected = m_pwi->focuserIsConnected();

    m_connectBtn->setDisabled(isConnected);
    m_disconnectBtn->setEnabled(isConnected);
}

void FocusController::checkPwiExists() {
    if (m_pwi->focuserExists()) {
        m_mainLayout->setCurrentIndex(1);
    } else {
        m_mainLayout->setCurrentIndex(0);
        m_emergencyStopped = false;
    }
}

void FocusController::moveFocuser(float newPosition) {
    m_pwi->focuserGoto(newPosition);
}

FocusController::FocusController(QWidget *parent, IPWI4Client *pwi) : QWidget{parent}, m_pwi(pwi) {
    m_focuserId = getNextId();

    QWidget *notExistsContainer = new QWidget(this);
    QVBoxLayout *notExistsLayout = new QVBoxLayout(notExistsContainer);
    QLabel *label = new QLabel();
    label->setText(QString(("PWI Server not detected at \n" + m_pwi->clientEndpoint()).c_str()));
    label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    label->setFont(QFont("Consolas", 32));
    label->setStyleSheet("color: red;");
    notExistsLayout->addWidget(label);

    m_mainLayout = new QStackedLayout();
    m_mainLayout->addWidget(notExistsContainer);
    this->setupWidget();

    this->setLayout(m_mainLayout);
}

FocusController::~FocusController() {
    delete m_pwi;
}

// slots
void FocusController::doPwiConnect(bool isConnecting) {
    if (isConnecting) {
        m_pwi->focuserConnect();
        m_connectBtn->setDisabled(true);
    } else {
        m_pwi->focuserDisconnect();
        m_disconnectBtn->setDisabled(true);
    }
}

void FocusController::updatePosition() {
    if (m_pwi->focuserIsConnected()) {
        m_positionLabel->setText(QString::number(m_pwi->focuserPosition()));
    }
}

void FocusController::updateStepSize(const QString &textChanged) {
    float stepSizeChange = textChanged.toFloat();
    if (stepSizeChange > 500) {
        m_upStepBtn->setDisabled(true);
        m_downStepBtn->setDisabled(true);
        m_stepSizeText->setStyleSheet("background-color: darkred;");
    } else {
        m_upStepBtn->setDisabled(false);
        m_downStepBtn->setDisabled(false);
        m_stepSize = stepSizeChange;
        m_stepSizeText->setStyleSheet("background-color: none;");
    }
}

void FocusController::updateIsMoving() {
    if (m_pwi->focuserIsConnected()) {
        if (m_emergencyStopped) {
            m_isMovingLabel->setText("EMERGENCY STOP\nRestart PWI server");
            m_isMovingLabel->setStyleSheet("color: red");
            this->setEnabledControls(false);
        } else if (m_pwi->focuserIsMoving()) {
            m_isMovingLabel->setText("Moving...");
            m_isMovingLabel->setStyleSheet("color: red");
            this->setEnabledControls(false);
        } else {
            m_isMovingLabel->setText("Ready");
            m_isMovingLabel->setStyleSheet("color: green");
            this->setEnabledControls(true);
        }
    } else {
        m_isMovingLabel->setText("No connection...");
        m_isMovingLabel->setStyleSheet("color: gray;");
    }
}
