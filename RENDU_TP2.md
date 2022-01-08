# Compte rendu de TP 2

Composition du groupe : 
- Vincent COMMIN
- Louis LEENART

# Introduction

# Exercice 1 : Suppression d'un bruit impulsionnel

L'objectif de cet exercice est de retiré le bruit impulsionnel de cette image : ![](img/tp2/Rubiks_cube_L_impulse.png)

## 1. Filtre moyenneur
| ![](img/tp2/blured_1_3x3.png) | ![](img/tp2/blured_2_3x3.png) | ![](img/tp2/blured_6_3x3.png) |
| ----------------------------- | ----------------------------- | ----------------------------- |
| *image floutée 1 fois*        | *image floutée 2 fois*        | *image floutée 6 fois*        |

Après application du filtre moyenneur plusieurs fois sur l'image, on constate que celle-ci ne tend pas vers une image amélioré. Non-seulement l'image n'est pas bien débruité mais celle-ci devient de plus en plus floue. On perd donc toutes les informations liées au contour.

| ![](img/tp2/blured_1_5x5.png) | ![](img/tp2/blured_1_7x7.png) |
| ----------------------------- | ----------------------------- |
|   *image floutée 1 fois par un filtre 5x5*                            |  *image floutée 1 fois par un filtre 7x7*                             |

On remarque que même en changeant la taille du filtre, le résultat reste le même, à savoir : flou tout en diluant le bruit dans l'image.

## 2. Filtre gaussien



## 3. Filtre bilateral

## 4. Filtre médian

## 5. Histogramme avec masque de l'image bruitée

## 6. Filtre médian #2 

## 7. Plusieurs passes de filtre médian

## 8. Bruit blanc gaussien

