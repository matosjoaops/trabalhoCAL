#ifndef CAL_POSICAO_H
#define CAL_POSICAO_H

#include <cmath>

class Posicao {
private:
    double latitude, longitude;
public:
    Posicao(double latitude, double longitude);
    double getLatitude() const;
    double getLongitude() const;
    void setLatitude(double latitude);
    void setLongitude(double longitude);
    double calcDist(Posicao pos);
};


#endif //CAL_POSICAO_H