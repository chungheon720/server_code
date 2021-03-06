#include <iostream>
#include <olc_net.h>

enum class CustomMsgTypes : uint32_t
{
    FireBullet,
    MovePlayer
};
class CustomClient : public olc::net::client_interface<CustomMsgTypes>
{
    /*
    Example
    CustomClient c;
    int portNum = 1000;
    c.Connect("hostname", portNum);
    c.FireBullet(1.0f, 2.0f)
    */
    bool FireBullet(float x, float y)
    {
        olc::net::message<CustomMsgTypes> msg;
        msg.header.id = CustomMsgTypes::FireBullet
        msg << x << y;
        Send(msg);

    }
};
int main()
{

    return 0;
}
