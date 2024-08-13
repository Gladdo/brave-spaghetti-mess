#include <assert.h>
#include "linmath.h"

namespace physic{

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                     3D MATHEMATIC ELEMENTS DEFINITIONS
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // ====================================================================================
    // Vector

    #define DOT *
    #define CROSS ^

    struct vec{
        float x;
        float y;
        float z;

        vec(){
            x = 0;
            y = 0;
            z = 0;
        }

        vec(float vx, float vy){
            x = vx;
            y = vy;
            z = 0;
        }

        vec(float vx, float vy, float vz){
            x = vx;
            y = vy;
            z = vz;
        }

        vec(vec& v){
            x = v.x;
            y = v.y;
            z = v.z;
        }

        vec(vec&& v){
            x = v.x;
            y = v.y;
            z = v.z;
        }

        vec& operator=(vec&& v){
            x = v.x;
            y = v.y;
            z = v.z;
        }

        float operator*(const vec& v){
            float res;
            res = x * v.x + y * v.y + z * v.z; 
        }
        
        vec operator^(const vec& v){
            vec res;
            res.x = y * v.z - z * v.y;
            res.y = - ( x * v.z - z * v.x );
            res.z = x * v.y - y * v.x;
            return res;
        }

        vec operator*(const float s){
            vec res;
            res.x = x * s;
            res.y = y * s;
            res.z = y * s;
            return res;
        }

        vec operator+(const vec& v){
            vec res;
            res.x = x + v.x;
            res.y = y + v.y;
            res.z = z + v.z;
            return res;
        }

        vec operator-(const vec& v){
            vec res;
            res.x = x - v.x;
            res.y = y - v.y;
            res.z = z - v.z;
            return res;
        }

        float length(){
            return vec3_len(vec3 {x, y, z});
        }

        void normalize(){
            vec3 res;
            vec3_norm(res, vec3 {x, y, z});
            x = res[0];
            y = res[1];
            z = res[2];
        }
    };

    // ====================================================================================
    // Matrix

    struct mat{
        mat4x4 matrix;

        // ====================================================================================
        // Constructor:
        // Inizializza la matrice ad un'array di zeri.
        mat(){
            for(int i=0; i < 4; i++)
                for(int j=0; j < 4; j++)
                    matrix[i][j] = 0;            
        }
        
        // ====================================================================================
        // Move Constructor:
        // Definisce come gestire la costruzione di una mat quando si usa una istruzione del tipo:
        //      mat matrix ({ mat() });
        // Ovvero quando l'argomento al costruttore è un oggetto mat temporaneo.
        // Inoltre viene utilizzato quando una funzione ritorna un oggetto di tipo mat:
        //      mat foo(){
        //          mat tmp;
        //          // manipulate tmp
        //          return tmp;
        //      }
        // Infatti quando foo() appare in un'espressione, il token "foo()" equivale a una mat. 
        // Il valore di questa mat deriva dal valore di tmp che era precedentemente sullo stack 
        // e da cui si è fatto il move:
        //      mat m = foo();
        // equivale a
        //      mat m = move(tmp);
        mat(mat&& moved_m){
            mat4x4 identity;
            mat4x4_identity(identity);
            mat4x4_mul(matrix, identity, moved_m.matrix);
        }

        // ====================================================================================
        // Move Assignement:
        // Definisce il comportamento dell'istruzione "m = moved_m;" quando moved_m è un r_value.
        // Copia in m la matrice contenuta in moved_m.
        mat& operator=(mat&& moved_m){
            mat4x4 identity;
            mat4x4_identity(identity);
            mat4x4_mul(matrix, identity, moved_m.matrix);
            return *this;
        }

        // ====================================================================================
        // Product operator overload

        // Matrix * Vector
        vec operator*(const vec& v){
            vec4 tmp {v.x, v.y, v.z, 1};
            vec4 tmp_res;
            mat4x4_mul_vec4(tmp_res, matrix, tmp);

            return {tmp_res[0], tmp_res[1], tmp_res[2]};
        }

        // Matrix * Vector
        mat operator*(const mat& m){ 
            mat res;
            mat4x4_mul(res.matrix, matrix, m.matrix);
            return res;
        }

        // ====================================================================================
        // Set and Getters for matrix elements

        void set_element(int row, int column, float value){
            if(row < 4 && column < 4)
                matrix[row][column] = value;
        }

        float get_element(int row, int column){
            if(!(row < 4 && column < 4))
                assert("Row or column value out of bound for mat element!");
            return matrix[row][column];
        }
        
        void set_to_identity(){
            
        }
        
    };

    mat create_model_matrix(float x, float y, float z){
       mat res;
       mat4x4_identity(res.matrix);
       res.set_element(0,3, x);
       res.set_element(1,3, y);
       res.set_element(2,3, z);
       return res;
    }
}