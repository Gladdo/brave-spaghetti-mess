#include "stb_image.h"

#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>

std::vector<unsigned char> load_image_to_unsigned_char_vector(std::string filename, int* img_width, int* img_height){
    
    std::vector<unsigned char> vec;
    int nrChannels;

    unsigned char* data = stbi_load( filename.c_str(), img_width, img_height, &nrChannels, 0 );

    for(int i = 0; i < (*img_width)*(*img_height)*nrChannels; i++){
        vec.push_back(data[i]);
    }

    stbi_image_free(data);

    return vec;
}

std::string load_multiline_txt_to_string(std::string filename){
    
    std::string text;

    //Open file
    std::ifstream input_file(filename);

    //Check if opened successfully
    if(!input_file.is_open()){
        std::cerr << "Error opening file " << filename << std::endl << std::flush;
        return "";
    }

    std::string input_line; 
    //Read the file line by line
    while(std::getline(input_file, input_line)){
        text += input_line + "\n";
    }

    return text;

}

using namespace std;

/* Osservazione sul valore di ritorno:
A prescindere dalle opzioni, per tornare un'array da una funzione il meccanismo è il seguente:
- Faccio un malloc nella funzione
- Ritorno il puntatore all'area di memoria
Le opzioni sono quindi due:
- Naked: Fare questi step in modo esplicito, passando all'utente un puntatore con la responsabilità di gestire
l'area di memoria
- Vector: usare un vector; questo si prende carico di fare la malloc per noi, fare in move constructor quando
è utilizzato il valore di ritorno, e in generale far si che l'area di memoria contenenti i valori flaot sia vincolato
allo span della variabile vector stessa!
*/

vector<float> load_txt_to_float_vector(string filename){

    vector<float> vec;

    //Open file
    ifstream input_file(filename);

    //Check if opened successfully
    if(!input_file.is_open()){
        cerr << "Error opening file " << filename << endl << flush;
        return vec;
    }

    string line; 

    //Read the file line by line
    while(getline(input_file, line)){
        std::istringstream words(line);
        std::string word;
        //Read the line word by word
        while(words >> word){
            try{
                vec.push_back(stof(word));
            }catch(const invalid_argument& ia){ 
                cerr << "Invalid argument " << ia.what() << " while reading file " << filename << endl << flush;
            }
        }
    }

    return vec;
}