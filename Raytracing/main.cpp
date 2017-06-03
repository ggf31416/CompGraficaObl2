#include <iostream>
#include "vmath.h"
#include <ctime>

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
    float coef_difuso;
    float exp_especular;
    float indice_refr;

    bool esReflejante;
    bool esTransparente;
};

Luz** luces;

COLOR_F traza_RR(const VEC& rayo,const VEC& V,int profundidad);
COLOR_F sombra_RR(Interseccion& inter,const VEC& dir_rayo, const VEC& V,const VEC& normal, int profundidad);

// Release
//Normalize double:
//normalize (div): 37364 - 3.4641e+009
//normalize (inv): 18351 - 3.4641e+009
//Normalize float:
//normalize (div): 37368 - 3.4641e+009
//normalize (inv): 18353 - 3.4641e+009

// Release -O3
//Normalize double:
//normalize (div):  37374 - 3.4641e+009
//normalize (inv): 18410 - 3.4641e+009
//Normalize float:
//normalize (div):  37364 - 3.4641e+009
//normalize (inv): 18361 - 3.4641e+009


// Debug
//Normalize double:
//normalize (div):  126926 - 3.4641e+009
//normalize (inv): 108589 - 3.4641e+009
//Normalize float:
//normalize (div):  50530 - 3.35544e+007
//normalize (inv): 40439 - 3.35544e+007

template<typename T> void speedNormalize3(){
    int MAX = 2000000000;
    T acum = 0;
    clock_t time1 = clock();
    for(int i = 1; i <= MAX; i++){
        Vector3<T> n(i,i+100,i+200);
        n.normalize_div();
        acum += n.x + n.y + n.z;
    }
    float ms = (1000.0 * (clock() - time1)) / CLOCKS_PER_SEC;
    cout << "normalize (div): " << ms << " - " << acum << endl;
    time1 = clock();
    acum = 0;
    for(int i = 1; i <= MAX; i++){
        Vector3<T> n(i,i+100,i+200);
        n.normalize();
        acum +=  n.x + n.y + n.z;
    }
    ms = (1000.0 * (clock() - time1)) / CLOCKS_PER_SEC;
    cout << "normalize (inv): " << ms << " - " << acum << endl;

}

int main()
{
    /*cout << "Normalize double:" << endl;
    speedNormalize3<double>();
    cout << "Normalize float:" << endl;
    speedNormalize3<float>();*/
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

// S_i , determina que fraccion de la luz llega desde la fuente de luz al punto P
float CalcularSombra(const VEC& P,Luz* luz){
    return 1;
}

const float ATT_C1 = 0; // factor cte
const float ATT_C2 = 0; // factor dl
const float ATT_C3 = 1; // factor dl^2
float Atenuacion(const VEC& vector_s){
    float d2 = vector_s.lengthSq();
    float d1 = ATT_C2 > 0 ? ATT_C2 * sqrt(d2) : 0;
    return min(1 / (ATT_C1 + d1 + ATT_C3 * d2),1.0f);
}

/*
//inter: Datos de interseccion
//dir_rayo: Vector direccion del rayo (normalizado)
//V: Punto de origen del rayo
//normal: Vector normal a la superficie (normalizado)
//profundidad: nivel recursivo
*/
COLOR_F sombra_RR(Interseccion& inter,const VEC& dir_rayo, const VEC& V,const VEC& normal, int profundidad){
    COLOR_F color = colorAmbiente();
    Objeto* obj = getObjeto(inter.objetoIdx);
    for(int l = 0; l < CANT_LUCES; l++){
        Luz* luz = luces[l];
        // rayo desde punto a luz
        VEC vector_s = inter.punto - luz->posicion; // corregir formula
        VEC dir_s = vector_s.normalized();

        double dot = normal.dotProduct(dir_rayo);
        if (dot > 0){
            // calcular cuanta luz es bloqueada por sup. opacas y transparentes y usarlas para...
            double S = CalcularSombra(inter.punto,luz);
            if (S > 0){ // si el punto no esta en sombra

                float f_att = Atenuacion(vector_s);
                COLOR_F color_luz = S * f_att * luz->color;

                COLOR_F color_dif = obj->getColorDifusoAt(inter.punto_obj) *  obj->coef_difuso * dot ;

                VEC rayo_r = getRayoReflexion(dir_rayo,normal);

                float phong = getValorPhong(rayo_r,V,obj->exp_especular);
                COLOR_F color_espec = obj->getColorEspecularAt(inter.punto_obj) * obj->coef_especular * phong;

                color += color_luz * (color_dif + color_espec); // producto elemento a elemento

            }

        }
    }
    if (profundidad < MAX_RECURSION_LEVEL){

        if (obj->esReflejante){
            // rayo en la direccion de reflexion desde punto
            VEC rayo_r = getRayoReflexion(dir_rayo,normal);
            COLOR_F color_r = traza_RR(rayo_r,inter.punto,profundidad + 1);
            color += color_r * obj->coef_especular;
        }
        if (obj->esTransparente){
            bool noTIR = true; // si NO ocurre la Reflexion Interna Total (TIR)
            if (noTIR){
                // rayo en la direccion de refraccion desde punto;
                VEC rayo_t;
                COLOR_F color_t = traza_RR(rayo_t,inter.punto,profundidad + 1);
                color += color_t * obj->coef_transmision;
            }
        }
    }
    return color;
}


COLOR_F traza_RR(const VEC& rayo,const VEC& V,int profundidad){
    // determinar la intersección más cercana de rayo con un objeto
    Interseccion inter;
    bool intersecto = interseccion(rayo,&inter);
    if (intersecto){
        // calcular la normal en la interseccion
        VEC normal = getNormal(&inter);
        return sombra_RR(inter,rayo,V,normal,profundidad);
    }
    return clearColor;
}


void raytracing(int w, int h){
    VEC pos;
    for(int i = 0; i < h; i++){ // fila
        for(int j = 0; j < w; j++){ // columna
            // determinar rayo por centro de proyeccion y pixel
            Vector3d rayo;
            COLOR_F p = traza_RR(rayo,pos,1);
        }
    }
}


