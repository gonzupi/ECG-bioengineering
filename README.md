El programa ECG-PL.cpp funciona en 3 pasos:
  1 - Copia los datos del fichero a dos LISTAS
  2 - Hace la FFT de esas listas, junto con algunos pasos para facilitar el cálculo
  3 - Coge el máximo de la FFT de cada ventana y calcula su media.

Por defecto funciona con el archivo Gonzy, pero puedes usar otro archivo
escribiendo su nombre después del comando (deben estar en la misma carpeta)

Si se quiere compilar usar el de la carpeta códigoCompleto, ya que hace falta la biblioteca para fourier.
Sería así:

   $ g++ kiss_fft.c ECG-PL.cpp -o ECG_Analycer

para ejecutar sería desde un terminal linux así:
  
   ./ECG_Analycer [opcional el nombre de otro archivo]


 - Diferencias entre fotopletismografía y ECG
Salen diferentes en función de la ventana que cojas, puesto que, al haber retraso, el analisis no cuadra con exactitud.
Ademas, la fotopletismografía me sale a la mitad de frecuencia que la ECG por algún motivo que no llego a entender.

 - ¿Como mejorar el algoritmo?
Se podría ajustar la ventana automáticamente en función del ruido o de la comparción en función de las dos medidas.
También se podría mejorar el uso de las dos entradas para conseguir un mejor resultado, teniendo en cuenta el desfase que hay entre las dos señales y los problemas de la ventana de tiempos.

 - Uso las librerías kiss_fft que facilitan muchísimo el trabajo y ya que no encuentro la referencia las subo aquí
 
