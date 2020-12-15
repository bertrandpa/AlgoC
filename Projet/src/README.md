# Difficulté
* Le passage de fonction par paramètre pour le calcule

# Commentaires
* Pour le calcule, choix de faire au style fonctionnel pour mieux factoriser
* La lib math.h n'a pas été utilisée
* Le format double ajoute des zéros et pollue l'échange json, on a donc ajouter une fonction qui "trim" les zéros avant envoie
* Des fuites mémoires ont été trouvées sur le code de base (+ de 1Mo pour l'envoie des couleures depuis une image bmp) et fixé (avec Valgrind)
