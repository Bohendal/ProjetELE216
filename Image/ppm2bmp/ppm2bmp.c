#include <stdio.h>
#include <stdint.h>

#include "imageppm.h"
#include "imagebmp.h"

//ppm to bmp:
/*
int main(int argc, char *argv[]){
    if (argc == 3){
        image_ppm_t *img_source = image_ppm_du_fichier_ppm(argv[1]);
        if (image_ppm_est_valide(img_source)){
            printf("image %s est valide\n", argv[1]);
        } else{
            printf("image %s est invalide\n", argv[1]);
        }
    }else {
        printf("Usage : ppm2bmp fichierPPM fichierBMP\n");
    }
    return 0;
}
*/

/*
int main(int argc, char *argv[]) {

    if (argc == 3) {
        image_ppm_t *image_source = image_ppm_du_fichier_ppm(argv[1]);
        if (image_ppm_est_valide(image_source)) {
            image_bmp_t *image_dest = image_bmp_de_image_ppm(image_source);
            image_bmp_sauvegarde_fichier(image_dest, argv[2]);           
            return 0;
        } else {
            return 1;
        }
    } else {
            printf("Usage : ppm2bmp fichierPPM fichierBMP\n");
            return 0;
    }
}
*/
//bmp2ppm:
int main(int argc, char *argv[]) {

    if (argc == 3) {
        image_bmp_t *image_source = image_bmp_du_fichier_bmp(argv[1]);
        if (image_bmp_est_valide(image_source)) {
            image_ppm_t *image_dest = image_ppm_de_image_bmp(image_source);
            image_ppm_sauvegarde_fichier(image_dest, argv[2]);           
            return 0;
        } else {
            return 1;
        }
    } else {
            printf("Usage : bmp2ppm fichierBMP fichierPPM\n");
            return 0;
    }
}


