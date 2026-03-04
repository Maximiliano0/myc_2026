// Librerías
#include <stdlib.h>
#include <stdio.h>

int main (void){
    int y = 4;
    int k = 10;
    int x; 
    // int y = 4, k = 10, x; 
    float y2 = 4, k2 = 10, x2;

    // Sin problemas
    x = k * y + 2; 
    printf("Sin parentesis x = %d\n", x);
    
    x = (k * y) + 2; 
    printf("Con parentesis x = %d\n", x);
    
    // Con problemas
    x = y / 2 * k;
    printf("Division sin parentesis x = %d\n", x);

    // Corregido pero con casteo implicito
    x = y / (2 * k);
    printf("Division con parentesis x = %d\n", x);

    // Corregido con casteo explicito
    x2 = y2 / (2 * k2);
    printf("Division con parentesis y casteo explicito x = %.2f\n", x2);

    // Corrigiendo todo
    x2 = ((float) y) / (2 * k);
    printf("Corrigiendo todo x = %.2f\n", x2);

    return 0;
}