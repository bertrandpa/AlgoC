# Difficulté
* Les tests n'ont pas été finis (intégration dure à mettre en place)
* Plusieurs idées testées pour le multi-client : fork puis thread et enfin select qui fonctionne
* Le fait d'avoir voulue rendre le client plus lourd (menu, saisie à la volé des valeurs...) rend plus difficile les tests.
# Commentaires
* Format accepté des msg : 
- message/nom : alphanum avec espace
- calcule : 1 ou 2 opérandes pour + - * /
- couleurs : pour utiliser le path il faut faire ./client pathToImage et sélectionner l'option couleur dans le menu. Pour le format entré à la main : 6 char en hexa
- balise : alphabétique seuelement
