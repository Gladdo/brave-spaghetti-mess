#include <assert.h>
#include <array>
#include "linmath.h"

/* namespace physic{

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

    typedef std::array<float, 16> mat;

    void mat_to_mat4x4(mat4x4& m4x4, mat& m){
        for(int i = 0; i < 4; i ++)
            for(int j = 0; j < 4; j++)
                m4x4[i][j] = m[i*4+j];
    }

    void mat4x4_to_mat(mat4x4& m4x4, mat& m){
        for(int i = 0; i < 4; i ++)
            for(int j = 0; j < 4; j++)
                m[i*4+j] = m4x4[i][j];
    }

    struct mat{
        float matrix [16];

        // ====================================================================================
        // Constructor:
        // Inizializza la matrice ad un'array di zeri
        mat(){
            for(int i=0; i < 16; i++)         
                matrix[i] = 0;
        }

    private:

        void array_to_mat4x4(mat4x4& m_out){
            for(int i = 0; i < 4; i ++)
                for(int j = 0; j < 4; j++)
                    m_out[i][j] = matrix[i*4+j];
        }

        void mat4x4_to_array(mat4x4& m_in){
            for(int i = 0; i < 4; i ++)
                for(int j = 0; j < 4; j++)
                    matrix[i*4+j] = m_in[i][j];
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
                matrix[row*4 + column] = value;
        }

        float get_element(int row, int column){
            if(!(row < 4 && column < 4))
                assert("Row or column value out of bound for mat element!");
            return matrix[row * 4 + column];
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
} */