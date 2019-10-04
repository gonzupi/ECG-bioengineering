//Programa creado por : Gonzalo Bueno Santana
////////////////////////////////////////////////////////////////////////////////
/*

Este programa funciona en 3 pasos:
  1 - Copia los datos del fichero a dos LISTAS
  2 - Hace la FFT de esas listas, junto con algunos pasos para facilitar el cálculo
  3 - Coge el máximo de la FFT de cada ventana y calcula su media.

Por defecto funciona con el archivo Gonzy, pero puedes usar otro archivo
escribiendo su nombre después del comando (deben estar en la misma carpeta)

Para compilarlo (ya está compilado en la carpeta pero bueno,
hay que añadir la biblioteca asink lo dejo pa que no se me olvide):
  g++ kiss_fft.c AnalisisGonzy_Final.cpp -o ProgramaFinal

para ejecutar sería así:
  ./ProgramaFinal [opcional el nombre de otro archivo]

*/


#include <iostream>
#include <fstream>
#include <list>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "kiss_fft.h"
#define COMENTARIOS 1//Activa/desactiva los comentarios sobre los calculos en cada ventana. Deja ver la tasa cardiaca instantanea.
#define DEPURACION 0 // Puesto a 1 activa diversos cout para depurar el código y encontrar fallos. a 0 desactiva

using namespace std;


int main(int argc,char *argv[] ){
   double temp1, temp2;//Variables auxiliares
   int anchoVentana, frecMedida;//Tiempo de cada ventana de tiempo para la tasa cardiaca instantanea,
                                //frecuencia de la medida que vamos a establecer para acelerar el cálculo.
   char cadena[16];//Variable auxiliar para la copia de datos a las listas
   list<double> listaDatosECG;//listas de datos
   list<double> listaDatosP;
   list<double> listaDatosMixtaP, listaDatosMixtaECG;
   list<double> auxiliar;//Cadenas auxiliares
   list<double> auxiliar2;
   list<double> auxiliar3;
   list<double> auxiliar4;

   ifstream fichero;//Fichero de lectura

   if(argc==1){
       cout << endl << "No has introducido ningún nombre de archivo, selecciono por defecto Gonzy" << endl << endl;
       fichero.open("Gonzy"); //Abrimos el fichero de datos gonzy (cambiar el nombre para probar otro archivo)
       // donde están los datos de las lecturas en las columnas 2 y 3.
   }else if(argc==2){
      fichero.open(argv[1]);//Escribimos el nombre?
      if(fichero.is_open()==0){
         cout << "ERROR CON EL NOMBRE DEL FICHERO" << endl;
         exit(-1);
      }
   }else{
     cout << "ERROR - Introduzca el nombre del programa seguido del del fichero" << endl;
     return(0);
   }

   cout << "\nVamos a calcular las ppm analizando una ventana de tiempo definida por el usuario.\n" << "Introduce el ancho de la ventana (en segundos) (recomendado 15):\t";
   cin >> anchoVentana;
   cout << endl;

   do{
     cout << "\nIntroduce el sesgo de calidad para acelerar el proceso, establece la frecuencia de medida en hercios (máximo 1000) (recomendados 200):\t";
     cin >> frecMedida;
   }while(frecMedida<=0 || frecMedida>1000);

   int aux=0;//Auxiliar para la cuenta de elementos, FILA
   int aux2=0;//Auxiliar para la cuenta de en qué fila estoy.
   char* pEnd;//Pte decimal del dato

   while(!fichero.eof()){//Hasta el fin del fichero
     if(aux==0){//El primer dato lo tiramos a la basura.
           fichero >> cadena;
           aux++;
     }else{//El segundo y el tercer dato,  trabajamos con ellos
           if(aux2 >= (1000/frecMedida) && !fichero.eof()){ //Hacemos el filtro de datos, cogemos uno de cada varios datos, disminuyendo la frecuencia de muestreo y tirando datos.
                   //Guardamos los datos en una cadena temporal, los mostramos si DEPURACION
                   fichero >> cadena;
                   if(DEPURACION) cout << "ECG dato Original #" << cadena <<"\t";

                   //Pasamos la cadena a double y lo añadimos a la lista
                   temp2=strtod(cadena, &pEnd);
                   pEnd[0]='.';//A c++ no le gustan las comas, prefiere puntos en los decimales.
                   temp1 = atof(pEnd);
                   if(cadena[0]=='-'){
                     temp2=temp2-temp1;
                   }else{
                     temp2=temp2+temp1;
                   }
                   listaDatosECG.push_back(temp2);
                   if(DEPURACION) cout <<"Dato en la lista #" << listaDatosECG.back() << endl;
                   //Guardamos los datos en una cadena temporal, los mostramos si DEPURACION
                   fichero >> cadena;
                   if(DEPURACION) cout << "Pleti Dato Original #" << cadena << "\t";
                   //Pasamos la cadena a double y lo añadimos a la lista
                   temp2=strtod(cadena, &pEnd);
                   pEnd[0]='.';
                   temp1 = atof(pEnd);
                   if(cadena[0]=='-'){
                     temp2=temp2-temp1;
                   }else{
                     temp2=temp2+temp1;
                   }
                   listaDatosP.push_back(temp2);
                   if(DEPURACION) cout << "Dato en la lista #" << listaDatosP.back() << endl;

                   //Incrementamos el los auxiliares para despreciar datos y para la cuenta.
                   aux2=0;//Empezamos a contar filas again
                   aux=0;//Volvemos al primer dato
           }else{//Los ficheros que no nos interesan los descartamos.
                   fichero >> cadena;//Tiramos los datos que no me importan por el sesgo.
                   fichero >> cadena;
                   aux=0;//volvemos al primer dato
                   aux2++;//incremento fila
           }
     }
   }//Hasta aquí se han copiado los datos de interés en las listas DatosP y DatosECG
   listaDatosMixtaP=listaDatosP;
   listaDatosMixtaECG=listaDatosECG;
////////////////////////////////////////////////////////////////////////////////////////////////////////////
if(COMENTARIOS) cout << endl << endl << "--------------------------LISTAS COPIADAS-------------" << endl << endl<< endl << endl;

//fft - Vamos con la transformada rápida de fourier.
int N=(frecMedida*anchoVentana+1);//Cuantas muestras hay? frecuencia por tiempo + 1 pa porsi el array necesita un menos o algo.
kiss_fft_cpx inECG[N], outECG[N];//Entrada y salida de ECG para fourier
kiss_fft_cpx inP[N], outP[N];//Para photopletismo.

size_t i;
i=0;
int tam;
tam = listaDatosP.size()/N;

//fotopletismografia
if(COMENTARIOS)   cout << endl << endl;
if(COMENTARIOS) cout << "--------------------------FOTOPLETISMOGRAFIA---------------" << endl<< endl << endl;
/////////////////////////////////////////////////////////////////////////////////////////////////////
kiss_fft_cfg cfgP;
int x=0;
for(x=0; x < tam ;x++){//Recorre todas las ventanas
       i=0;
       if(DEPURACION)cout<< "Valores 0 y N-1 FFT fotopletismografia" << endl;
       for (i = 0; i < N; i++){//Copio los datos de la ventana actual
         inP[i].r = listaDatosP.front();
         listaDatosP.pop_front();
         inP[i].i = 0;
       }
       if(DEPURACION) cout << "#" << inP[0].r << "\t"  <<  inP[N-1].r << endl ;

       if(DEPURACION) cout << endl << endl;

       if ((cfgP = kiss_fft_alloc(N, 0/*is_inverse_fft*/, NULL, NULL)) != NULL){//Hay memoria para la fft?
         kiss_fft(cfgP, inP, outP);
         kiss_fft_free(cfgP);//Limpio
         int max, maxAnt;
         max=0;
         maxAnt=0;
         int posMax, posMaxAnt;
         posMax=0, posMaxAnt=0;
         i=0;//Por algún motivo no me funciona en el for
         for (i = 0; i < N; i++){//Recorro la ventana buscando el máximo
              //Para ver los datos de la fft descomentar abajo
              //cout << "out[" << i << "]\t" << abs(outP[i].r) << endl;
              //cout << "in[" << i << "]\t" << inP[i].r << endl;

              if(max<abs(outP[i].r) && i>25 && i <350){//Filtro los HZ que claramente no son. Creo que las salidas son todas reales pero bueno, pongo abs pa porsi.
                maxAnt=max;
                max = abs(outP[i].r);
                posMaxAnt=posMax;//Detección de errores.
                posMax=i;
              }
         }
         if(COMENTARIOS) cout << "El valor máximo de la ventana "<<x<<" está en la posición [" << posMax<< "] y es de: " << max << endl;//Tasa cardiaca instantanea
         if(COMENTARIOS) cout << "El siguiente valor máximo está en la posición [ "<<posMaxAnt <<" ] y es de: "<<maxAnt << endl;
         if(abs(maxAnt - max) >10*(max)/100){//ruido?
           if(COMENTARIOS) cout << "La tasa cardiaca instantanea de este fragmento es : " << 2*posMax << endl <<endl;
           auxiliar.push_front(posMax);
         }else{//ruido
            if(COMENTARIOS) cout << "ERROR -> RUIDO" << endl <<endl;
         }
       }else{//Ha fallado fourier?
        if(COMENTARIOS) printf("No hay memoria suficiente?\n");
        exit(-1);
      }
  //////////////////////////////////////////////////////////////////////////////////////////////////
  }//for que recorre todas las ventanas
  //fin de la fotopletismografia


  //Inicio del ECG
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
  kiss_fft_cleanup();
  kiss_fft_cfg cfgECG;
  if(COMENTARIOS) cout << endl << endl<< endl << endl;
  if(COMENTARIOS) cout << "--------------------------ECG---------------" << endl<< endl<< endl;
  x=0;
  for(x=0; x < tam ;x++){
        if(DEPURACION) cout << "# Valores 0 y N-1 FFT ECG #" << endl ;
        for (i = 0; i < N; i++){//Copio los datos de la ventana actual
          //if(DEPURACION) cout << "Entro en el for2 - " << i << endl;
          inECG[i].r = listaDatosECG.front();
          listaDatosECG.pop_front();
          inECG[i].i = 0;
        }
        if(DEPURACION) cout << "#" << inECG[0].r << "\t"  <<  inECG[N-1].r << endl ;

        if(DEPURACION) cout << endl << "Creo furier ECG" << endl << endl;
        if ((cfgECG = kiss_fft_alloc(N, 0/*is_inverse_fft*/, NULL, NULL)) != NULL){//Hay memoria para la fft?
              kiss_fft(cfgECG, inECG, outECG);


              kiss_fft_free(cfgECG);//Limpio
              //if(DEPURACION) cout << "Libero memoria" << endl;
              int max, maxAnt;
              max=0;
              maxAnt=0;
              int posMax, posMaxAnt;
              posMax=0, posMaxAnt=0;
              i=0;//Por algún motivo no me funciona en el for
              for (i = 0; i < N; i++){//Recorro la ventana buscando el máximo
                   //Para ver los datos de la fft descomentar abajo
                    //if(DEPURACION) cout << "out[" << i << "]\t" << abs(outECG[i].r) << endl;
                  //  if(DEPURACION) cout << "in[" << i << "]\t" << inECG[i].r << endl;

                   if(max<abs(outECG[i].r) && i>25 && i <280){//Filtro los HZ que claramente no son.
                     maxAnt=max;
                     max = abs(outECG[i].r);
                     posMaxAnt=posMax;//Detección de errores.
                     posMax=i;
                   }
              }
            if(COMENTARIOS)  cout << "El valor máximo de la ventana "<<x<<" está en la posición [" << posMax<< "] y es de: " << max << endl;//Tasa cardiaca instantanea
            if(COMENTARIOS)  cout << "El siguiente valor máximo está en la posición [ "<<posMaxAnt <<" ] y es de: "<<maxAnt << endl;
              if((abs(maxAnt - max) >10*max/100)&&maxAnt!=0){//ruido?
                if(COMENTARIOS) cout << "La tasa cardiaca instantanea de este fragmento es : " << posMax << endl <<endl;
                auxiliar2.push_front(posMax);
              }else{//ruido
                if(COMENTARIOS) cout << "ERROR -> RUIDO" << endl <<endl;
              }
        }else{//Ha fallado fourier?
         if(COMENTARIOS) printf("No hay memoria suficiente?\n");
         exit(-1);
        }
}//for end

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //fin ECG

  //Inicio del algoritmo mixto
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
  kiss_fft_cleanup();
  if(COMENTARIOS) cout << endl << endl<< endl << endl;
  if(COMENTARIOS) cout << "--------------------------ALGORITMO MIXTO---------------" << endl<< endl<< endl;
  x=0;

  for(x=0; x < tam ;x++){
        if(DEPURACION) cout << "# Valores 0 y N-1 FFT Pleti y ECG #" << endl ;
        i=0;
        for (i = 0; i < N; i++){//Copio los datos de la ventana actual
          //if(DEPURACION) cout << "Entro en el for2 - " << i << endl;
          inECG[i].r = listaDatosMixtaECG.front();
          inP[i].r = listaDatosMixtaP.front();
          listaDatosMixtaECG.pop_front();
          listaDatosMixtaP.pop_front();
          inECG[i].i = 0;
          inP[i].i=0;
        }
        if(DEPURACION) cout << "ECG - #" << inECG[0].r << "\t"  <<  inECG[N-1].r << endl ;
        if(DEPURACION) cout << "P - #" << inP[0].r << "\t"  <<  inP[N-1].r << endl ;

        if(DEPURACION) cout << endl << "Creo furier ECG" << endl << endl;
        if ((cfgECG = kiss_fft_alloc(N, 0/*is_inverse_fft*/, NULL, NULL)) != NULL){//Hay memoria para la fft?
              kiss_fft(cfgECG, inECG, outECG);


              kiss_fft_free(cfgECG);//Limpio
              //if(DEPURACION) cout << "Libero memoria" << endl;
              int max, maxAnt;
              max=0;
              maxAnt=0;
              int posMax, posMaxAnt;
              posMax=0, posMaxAnt=0;
              i=0;//Por algún motivo no me funciona en el for
              for (i = 0; i < N; i++){//Recorro la ventana buscando el máximo
                   //Para ver los datos de la fft descomentar abajo
                    //if(DEPURACION) cout << "out[" << i << "]\t" << abs(outECG[i].r) << endl;
                  //  if(DEPURACION) cout << "in[" << i << "]\t" << inECG[i].r << endl;

                   if(max<abs(outECG[i].r) && i>25 && i <280){//Filtro los HZ que claramente no son.
                     maxAnt=max;
                     max = abs(outECG[i].r);
                     posMaxAnt=posMax;//Detección de errores.
                     posMax=i;
                   }
              }
            if(COMENTARIOS)  cout << "El valor máximo en la ECG de la ventana "<<x<<" está en la posición [" << posMax<< "] y es de: " << max << endl;//Tasa cardiaca instantanea
            //if(COMENTARIOS)  cout << "El siguiente valor máximo está en la posición [ "<<posMaxAnt <<" ] y es de: "<<maxAnt << endl;
              if((abs(maxAnt - max) >10*max/100)&&maxAnt!=0){//ruido?
                auxiliar3.push_front(posMax);
              }else{//ruido
                if(COMENTARIOS) cout << "#####  ERROR -> RUIDO: Hay dos o más frecuencias relevantes en la conversion a fourier, arriba más detalles  #####" << endl;
                auxiliar3.push_front(-1);
              }
        }else{//Ha fallado fourier?
         if(COMENTARIOS) printf("No hay memoria suficiente?\n");
         exit(-1);
        }

        if ((cfgP = kiss_fft_alloc(N, 0/*is_inverse_fft*/, NULL, NULL)) != NULL){//Hay memoria para la fft?
          kiss_fft(cfgP, inP, outP);
          kiss_fft_free(cfgP);//Limpio
          int max, maxAnt;
          max=0;
          maxAnt=0;
          int posMax, posMaxAnt;
          posMax=0, posMaxAnt=0;
          i=0;//Por algún motivo no me funciona en el for
          for (i = 0; i < N; i++){//Recorro la ventana buscando el máximo
               //Para ver los datos de la fft descomentar abajo
               //cout << "out[" << i << "]\t" << abs(outP[i].r) << endl;
               //cout << "in[" << i << "]\t" << inP[i].r << endl;

               if(max<abs(outP[i].r) && i>25 && i <350){//Filtro los HZ que claramente no son. Creo que las salidas son todas reales pero bueno, pongo abs pa porsi.
                 maxAnt=max;
                 max = abs(outP[i].r);
                 posMaxAnt=posMax;//Detección de errores.
                 posMax=i;
               }
          }
          if(COMENTARIOS) cout << "El valor máximo de la fotopletismografiade la ventana "<<x<<" está en la posición [" << 2*posMax<< "] y es de: " << max << endl;//Tasa cardiaca instantanea
          //if(COMENTARIOS) cout << "El siguiente valor máximo está en la posición [ "<<posMaxAnt <<" ] y es de: "<<maxAnt << endl;
          if(abs(maxAnt - max) >10*(max)/100){//ruido?
            auxiliar4.push_front(posMax);
          }else{//ruido
             if(COMENTARIOS) cout << "#####  ERROR -> RUIDO : Hay dos o más frecuencias relevantes en la conversion a fourier, arriba más detalles  #####" << endl;
             auxiliar4.push_front(-1);

          }
        }else{//Ha fallado fourier?
         if(COMENTARIOS) printf("No hay memoria suficiente?\n");
         exit(-1);
       }

       //Tasa instantanea mixta
       if(auxiliar4.front() != -1){
         if(auxiliar3.front() != -1){
           if(COMENTARIOS) cout << "La tasa cardiaca instantanea mixta de este fragmento es : " << ((2*auxiliar4.front()+auxiliar3.front())/2) << endl <<endl;

         }else{
           //A3 es -1
           if(COMENTARIOS) cout << "La tasa cardiaca instantanea mixta de este fragmento es : " << (2*auxiliar4.front()) << endl <<endl;
         }
       }else{
         //es -1
         if(COMENTARIOS) cout << "La tasa cardiaca instantanea mixta de este fragmento es : " << (auxiliar3.front()) << endl <<endl;
       }
}//for end

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //fin mixto


//Calculamos la media
 cout << endl << endl;
 temp1=0;
 temp2=0;
 int temp3;
 temp3=0;
 int temp4;
 temp4=0;

 tam= auxiliar.size();
 //Fotopletismo
 for(int x=0; x<tam;x++){
   temp1= temp1+auxiliar.front();
   auxiliar.pop_front();
   //cout << "temp "<<x<<" = #" << temp1 << "\t";
 }
 temp1=temp1/tam;
 if(DEPURACION) cout << "Media FOTOPLETISMOGRAFIA = " << temp1*tam << "/" << tam << endl;

 //ECG
 tam=auxiliar2.size();
 for(int x=0; x<tam;x++){
   temp2= temp2+auxiliar2.front();
   auxiliar2.pop_front();
 }
 temp2=temp2/tam;
 if(DEPURACION) cout << "Media ECG = " << temp2*tam << "/" << tam << endl;

 //MIXTO
 tam=auxiliar3.size();

 for(int x=0; x<tam;x++){
   if(auxiliar3.front()!=-1){
     if(auxiliar4.front()!=-1){
       temp3=temp3 + ((auxiliar3.front()+2*auxiliar4.front())/2);
     }else{//a4 -1
      temp3=temp3 + auxiliar3.front();
     }
   }else{//a3 -1
     if(auxiliar4.front()!=-1){
      temp3=temp3 + 2*auxiliar4.front();
     }else{
       //Este valor no se pilla, da error en los dos.
       temp4=temp4+1;
     }
   }
   auxiliar3.pop_front();
   auxiliar4.pop_front();
 }
 temp3=temp3/(tam-temp4);
 if(DEPURACION) cout << "Media mixta = " << temp3*tam << "/" << (tam-temp4) << endl;


 //No estoy seguro de porqué
 // las ppm en pletismo son la mitad que las de la ECG. He puesto un *2 pork cuadra.
 cout << "  La media final de las ppm según la fotopletismografia es de: "<<"\t[ " << 2*temp1 << " ] ppm" <<endl;
 cout << "  La media final de las ppm según la ECG es de: "<<"\t\t[ " <<temp2 << " ] ppm" <<endl;
 cout << "  La media final de las ppm según la ECG y la foto es de: "<<"\t[ " <<temp3 << " ] ppm" <<endl << endl;

 fichero.close();

 return 0;
}//main
