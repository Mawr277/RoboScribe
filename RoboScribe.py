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

#wykrywanie krawedzi
thresh_low = 100
thresh_high = 150

blur = cv2.GaussianBlur(img, (5,5), 0)
edges = cv2.Canny(blur, thresh_low, thresh_high)
contours, _ = cv2.findContours(edges, cv2.RETR_LIST, cv2.CHAIN_APPROX_NONE)


cv2.imshow("Krawędzie", edges)
cv2.waitKey(0)