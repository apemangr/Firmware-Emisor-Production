#include <stdlib.h>
#include "array_math.h"


/**
 * @brief Función de comparación para usar con qsort.
 *
 * @param a Puntero al primer elemento a comparar.
 * @param b Puntero al segundo elemento a comparar.
 * @return Un número negativo si a es menor que b, cero si son iguales,
 *         y un número positivo si a es mayor que b.
 *
 * Esta función de comparación se utiliza como argumento para la función qsort
 * para ordenar elementos en orden ascendente.
 * Se espera que a y b apunten a elementos del mismo tipo que se están comparando.
 */


int comparar(const void *a, const void *b) {
    return (*(uint16_t *)a - *(uint16_t *)b);
}

/**
 * @brief Calcula la mediana de un arreglo de números enteros sin signo.
 *
 * @param vector Arreglo de números enteros sin signo.
 * @param num Número de elementos en el arreglo.
 * @return La mediana del arreglo como un número entero sin signo.
 *
 * Esta función utiliza el algoritmo de ordenamiento quicksort para ordenar el
 * arreglo en orden ascendente y luego encuentra la mediana según el tamaño del arreglo.
 * Si el número de elementos es par, toma el promedio de los dos valores centrales;
 * si es impar, la mediana es el valor central.
 *
 * @note Esta función espera que el arreglo esté prellenado con datos y no modifica
 * el orden original de los elementos fuera de la función.
 */
uint16_t mediana(uint16_t *vector, uint16_t num)
{
  double mediana_d;  /**< Variable para almacenar la mediana como un número decimal de punto flotante */
  
  // Ordena el arreglo utilizando quicksort
  qsort(vector, num, sizeof(uint16_t), comparar);  


 /***** Utilizar este segmento para verificar que el vector este ordenado correctamente
  __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Arreglo despu�s de ordenar:\n");
  for (uint16_t i = 0; i < num; i++) {
  __LOG(LOG_SRC_APP, LOG_LEVEL_INFO,"%d \n ", vector[i]);
  }
*/

  if (num % 2 == 0) // Si hay un número par de elementos en el arreglo
  {
    uint16_t n = num / 2;
    // Si hay un número par de elementos, toma el promedio de los dos valores centrales
    mediana_d = (vector[n - 1] + vector[n] + vector[n - 2] + vector[n + 1]) / 4.0;
  }
  else // Si hay un número impar de elementos en el arreglo
  {
    uint16_t n = (num - 1) / 2;
    // Si hay un número impar de elementos, la mediana es el valor central
    mediana_d = (vector[n - 1] + vector[n] + vector[n + 1]) / 3.0;
  }

  // Devuelve la mediana como un número entero sin signo
  return (uint16_t)mediana_d;
}
