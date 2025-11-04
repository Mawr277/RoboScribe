#pip install numpy opencv-python
import numpy as np
import cv2 as cv2


#wczytanie obrazu
path = 'test.jpg'
img = cv2.imread(path) 
if img is None:
    print(f"Błąd: Nie udało się wczytać obrazu ze ścieżki: {path}")
    print("Sprawdź, czy plik istnieje i czy ścieżka jest poprawna.")
else:
    print("wczytano obraz")
    cv2.imshow("test", img)
    cv2.waitKey(0)
    cv2.destroyAllWindows