#include <iostream>
#include "vmath.h"

using namespace std;

int CANT_LUCES = 1;

// maximo nivel recursivo para raytracing
#define MAX_RECURSION_LEVEL 3
// vector usado normalmente
#define VEC Vector3d
// vector de baja precision
#define VEC_LOW Vector3f
// tipo usado para definir indice de objeto
#define IDX_OBJ int
// tipo usado para guardar el color en el resultado intermedio (con mayor precision que 8 bits o punto flotante)
#define COLOR_F Vector3f
// tipo usado para guardar cada componente de color (8 bits por ejemplo) en el resultado final
#define COLORVALUE unsigned char
struct Pixel{
    COLORVALUE r;
    COLORVALUE g;
    COLORVALUE b;

    Pixel(COLORVALUE _r,COLORVALUE _g, COLORVALUE _b){
        r = _r;
        g = _g;
        b = _b;
    }

    void assignFrom(const COLOR_F& c){
        float escala = 255;
        // redondear sin funcion round de c++11
        this->r = (COLORVALUE)((c.x * escala) + 0.5);
        this->g = (COLORVALUE)((c.y * escala) + 0.5);
        this->b = (COLORVALUE)((c.z * escala) + 0.5);
    }
};

COLOR_F clearColor;

//capaz es mejor pasarlo por referecia en vez de valor porque puede servir incluir mas datos
struct Interseccion{
    IDX_OBJ objetoIdx;
    VEC punto; // interseccion en cartesianas
    // interseccion en coordenadas del objeto, tipo a confirmar, segun sirva para acelerar los calculos
    VEC punto_obj;
};

struct Luz{
    COLOR_F color;
    VEC posicion;
    float intensidad;
};

class Objeto{
public:
    COLOR_F getColorDifusoAt(const VEC& punto){
        return VEC();
    }
    COLOR_F getColorEspecularAt(const VEC& punto){
        return VEC();
    }
    float coef_ambiental;
    float coef_transmision;
    float coef_especular;
    float exp_especular;
    float indice_refr;

    bool esReflejante;
    bool esTransparente;
};

Luz** luces;

COLOR_F traza_RR(const VEC& rayo,int profundidad);
COLOR_F sombra_RR(Interseccion& inter,const VEC& rayo, const VEC& normal, int profundidad);

int main()
{
    cout << "Hello world!" << endl;
    return 0;
}



bool interseccion(const VEC& rayo, Interseccion* inter){
    return false;

}

VEC getNormal(Interseccion* inter){
    return VEC(1,0,0);
}

COLOR_F colorAmbiente(){
    return COLOR_F(0,0,0);
}

Objeto* getObjeto(IDX_OBJ index){
    return NULL;
}

VEC getRayoReflexion(const VEC& L,const VEC& N){
    VEC R = 2 * N * (N.dotProduct(L)) - L;
    return R;
}


double getValorPhong(const VEC& r, const VEC& v, float exp){
    return pow(r.dotProduct(v),exp);
}

double getValorDifusa(const VEC& L,const VEC& N){
    return N.dotProduct(L);
}


bool refraccion(float n1, float n2,const VEC& incidente,const VEC& normal, VEC& T){
    // pag 286 Glassner
    bool TIR;
    float eta = n1 / n2;
    double c1 = -normal.dotProduct(incidente); // cos theta1
    double cs2 = 1 - eta * eta * (1 - c1 * c1 ); // cos^2 theta2
    if (cs2 < 0){
        // hubo tir
        TIR = true;
    }
    else{
        TIR = false;
        T = VecComb(eta,incidente,(eta * c1 - sqrt(cs2)),normal);//incidente * eta + normal * (eta * c1 - sqrt(cs2));
    }
    return TIR;
}



COLOR_F sombra_RR(Interseccion& inter,const VEC& rayo, const VEC& normal, int profundidad){
    COLOR_F color = colorAmbiente();
    for(int l = 0; l < CANT_LUCES; l++){
        Luz* luz = luces[l];
        // rayo desde punto a luz
        VEC rayo_s = inter.punto - luz->posicion; // corregir formula
        rayo_s.normalize();
        double dot = normal.dotProduct(rayo);
        if (dot > 0){
            // calcular cuanta luz es bloqueada por sup. opacas y transparentes y usarlas para...


        }
    }
    if (profundidad < MAX_RECURSION_LEVEL){
        Objeto* obj = getObjeto(inter.objetoIdx);
        if (obj->esReflejante){
            // rayo en la direccion de reflexion desde punto
            VEC rayo_r = getRayoReflexion(rayo,normal);
            COLOR_F color_r = traza_RR(rayo_r,profundidad + 1);
            color += color_r * obj->coef_especular;
        }
        if (obj->esTransparente){
            bool noTIR = true; // si NO ocurre la Reflexion Interna Total (TIR)
            if (noTIR){
                // rayo en la direccion de refraccion desde punto;
                VEC rayo_t;
                COLOR_F color_t = traza_RR(rayo_t,profundidad + 1);
                color += color_t * obj->coef_transmision;
            }
        }
    }
    return color;
}


COLOR_F traza_RR(const VEC& rayo,int profundidad){
    // determinar la intersección más cercana de rayo con un objeto
    Interseccion inter;
    bool intersecto = interseccion(rayo,&inter);
    if (intersecto){
        // calcular la normal en la interseccion
        VEC normal = getNormal(&inter);
        return sombra_RR(inter,rayo,normal,profundidad);
    }
    return clearColor;
}


void raytracing(int w, int h){
    for(int i = 0; i < h; i++){ // fila
        for(int j = 0; j < w; j++){ // columna
            // determinar rayo por centro de proyeccion y pixel
            Vector3d rayo;
            COLOR_F p = traza_RR(rayo,1);
        }
    }
}


