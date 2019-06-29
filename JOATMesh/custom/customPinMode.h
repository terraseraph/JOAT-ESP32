
/* Initialise a pin for output */
void customPin_init(uint8_t pinNo, bool input)
{
    if (input == true)
    {
        pinMode(pinNo, INPUT);
    }
    else
    {
        pinMode(pinNo, OUTPUT);
    }
}

void customPin_toggle(uint8_t pinNo, bool active)
{
    if (active == true)
    {
        pinMode(pinNo, OUTPUT);
        digitalWrite(pinNo, HIGH);
    }
    else
    {
        pinMode(pinNo, OUTPUT);
        digitalWrite(pinNo, LOW);
    }
}

/* For event action */
void processCustomPinAction(uint8_t pinNo, String action)
{
    if (action == "on" || action == "start")
    {
        customPin_toggle(pinNo, true);
    }
    else if (action == "off" || action == "stop")
    {
        customPin_toggle(pinNo, false);
    }
}
