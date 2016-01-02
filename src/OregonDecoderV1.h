class OregonDecoderV1 : public DecodeOOK {
public:
    OregonDecoderV1() {}

    virtual char decode (word width) {
        if (200 <= width && width < 1200) {
            byte w = width >= 700;
            switch (state) {
                case UNKNOWN:
                    if (w == 0)
                        ++flip;
                    else if (10 <= flip && flip <= 50) {
                        flip = 1;
                        manchester(1);
                    } else
                        return -1;
                    break;
                case OK:
                    if (w == 0)
                        state = T0;
                    else
                        manchester(1);
                    break;
                case T0:
                    if (w == 0)
                        manchester(0);
                    else
                        return -1;
                    break;
            }
            return 0;
        }
        if (width >= 2500 && pos >= 9)
            return 1;
        return -1;
    }
};
