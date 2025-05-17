#pragma once

class IPWI4Client { // interface
public:
    IPWI4Client() {}

    virtual void focuserConnect() = 0;
    virtual void focuserDisconnect() = 0;
    virtual void focuserEnable() = 0;
    virtual void focuserDisable() = 0;
    virtual void focuserGoto(float target) = 0;
    virtual void focuserStop() = 0;

    // properties
    virtual bool focuserIsConnected() = 0;
    virtual bool focuserIsEnabled() = 0;
    virtual bool focuserIsMoving() = 0;
    virtual float focuserPosition() = 0;

    virtual ~IPWI4Client() {}
};
