#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>

// struct jokaiselle yksittäiselle sanalle, johon talletetaan sana, kuinka monesti se esiintyy tekstissä
// sekä mahdolliset vasemmat ja oikeat lapset
struct node {
    unsigned long key;
    char word[100];
    int count;
    struct node *vasen;
    struct node *oikea;
};

struct node* newNode();
void addNode(struct node *node);
void countEsiintymiset(struct node *node, int *countArr);
void fillTopSata(struct node *node, struct node *tulosteArr[], int minValue, int *startIndex);
unsigned long hash(char *str);

struct node *rootNode;
int diffWordCount;
// Globaali muuttuja luettavalle sanalle
char sana[100];
// Seurataan kuinka monta kertaa eniten esiintyvä sana esiintyy
int maxCount = 0;


int main() {
    FILE* tiedosto;
    char ch;
    int n = 0;
    rootNode = newNode();
    rootNode -> key = 0;
    int wordCount = 0;
    diffWordCount = 0;
    char path[300];

    while (1) {
        printf("Name of the text file: ");
        fgets(path, 300, stdin);

        path[strlen(path)-1] = 0;

        tiedosto = fopen(path, "r");

        if (tiedosto != NULL) {
            printf("\n");
            break;
        }
        else {
            printf("File not found!\n");
        }
    }

    clock_t begin = clock();

    // Luetaan sanat tiedostosta ja tehdään jokaisesta sanasta uusi oma node
    while (ch != EOF) {
        ch = fgetc(tiedosto);

        if (isalpha(ch) || ch == '\'') {
            sana[n] = ch;
            n++;
        }
        else if (n > 0) {
            sana[n] = 0;
            n = 0;
            wordCount++;

            int i = 0;
            while(sana[i]) {
                sana[i] = toupper(sana[i]);
                i++;
            }

            addNode(newNode());
        }
    }

    // Lasketaan kuinka monta on x kertaa esiintyviä sanoja
    // maxCount on eniten esiintyneen sanan/sanojen esiintymiskerrat
    int countArr[maxCount+1];
    int startIndex[maxCount + 1];
    for (int i = 0; i < maxCount+1; i++) {
        countArr[i] = 0;
        startIndex[i] = 0;
    }

    countEsiintymiset(rootNode, countArr);

    // Selvitetään kuinka monella esiintymiskerralla pääsee TOP100aan
    // ja kuinka monta sanaa TOP100 sisältää kun otetaan yhtä monta
    // kertaa esiintyvät sanat huomioon
    int topSataCount = 0;
    int minValue = 0;
    for (int i = maxCount; i >= 0; i--) {
        if (countArr[i] > 0) {
            topSataCount += countArr[i];
            if (topSataCount >= 100) {
                minValue = i;
                break;
            }
        }
    }

    // Lasketaan jokaiselle x kertaa esiintyvälle sanalle mikä ko. sanan
    // aloitusindeksi on top100 taulukossa
    int kumulointi = 0;
    for (int i = minValue; i < maxCount + 1; i++) {
        if (countArr[i] > 0) {
            startIndex[i] = kumulointi;
            kumulointi += countArr[i];
        }
    }

    struct node *tulosteArr[topSataCount];

    for (int i = 0; i < topSataCount; i++) {
        tulosteArr[i] = NULL;
    }

    fillTopSata(rootNode, tulosteArr, minValue, startIndex);

    clock_t end = clock();

    // Lopputuloste
    printf("Aikaa meni %lf sekuntia\n", (double)(end - begin) / CLOCKS_PER_SEC);
    printf("Total number of words = %d\n", wordCount);
    printf("Number of different words = %d\n", diffWordCount);
    printf("The %d most common words:\n%-16s%s\n", topSataCount, "WORD", "NUMBER OF OCCURRENCES");

    for (int i = topSataCount - 1; i >= 0; i--) {
        printf("%-16s%d\n", tulosteArr[i] -> word, tulosteArr[i] -> count);
    }

    return 0;
}

// Luodaan uusi node ja palautetaan sen pointer
struct node* newNode() {
    struct node* node = (struct node*)malloc(sizeof(struct node));

    node -> key = hash(sana);

    for (int i = 0; i < strlen(sana); i++) {
        node -> word[i] = sana[i];
    }

    node -> word[strlen(sana)] = 0;

    node -> count = 1;
    node -> vasen = NULL;
    node -> oikea = NULL;

    return(node);
}

// Lisätään node binääripuuhun
void addNode(struct node *node) {
    if (rootNode -> key == 0) {
        rootNode = node;
        diffWordCount++;
    }
    else {
        struct node* currentNode = rootNode;

        while (1) {
            // Jos uuden noden avain on pienempi kuin tarkasteltavan solmun avain, niin
            // joka lisätään se tarkasteltavan solmun vasemmaksi lapseksi tai jatketaan
            // puun läpikäyntiä
            if (node -> key < currentNode -> key) {
                if (currentNode -> vasen == NULL) {
                    currentNode -> vasen = node;
                    diffWordCount++;
                    break;
                }
                else {
                    currentNode = currentNode -> vasen;
                }
            }
            // Jos noden avain suurempi, niin sama homma oikealle puolen
            else if (node -> key > currentNode -> key) {
                if (currentNode -> oikea == NULL) {
                    currentNode -> oikea = node;
                    diffWordCount++;
                    break;
                }
                else {
                    currentNode = currentNode -> oikea;
                }
            }
            // Sama hashtag
            else {
                currentNode -> count = currentNode -> count + 1;

                if (currentNode -> count > maxCount) maxCount = currentNode -> count;

                break;
            }
        }
    }
}

// Merkitään int taulukkoon kuinka monta kappaletta on sanoja jotka esiintyvät x kertaa
void countEsiintymiset(struct node *node, int *countArr) {
    if (node -> vasen != NULL) {
        countEsiintymiset(node -> vasen, countArr);
    }

    if (node -> oikea != NULL) {
        countEsiintymiset(node -> oikea, countArr);
    }

    countArr[node -> count] = countArr[node -> count] + 1;
}

/*
Lajitellaan sanat TOP100 (tai enemmän jos sanoja "jaetulla sijalla") taulukkoon.
Käydään kaikki nodet läpi ja jos sanan esiintymiskerrat riittävät TOP100 esiintymiseen
lisätään se taulukkoon. Sanan paikka taulukossa on selvitetty laskemalla kuinka monta
kertaa top100 sanat esiintyvät. Esimerkiksi jos vähiten esiintyneet sanat TOP100 taulukossa
ovat esiintyneet 7 kertaa ja niitä 30 kappaletta, niin TOP100 taulukkoa lähdetään täyttämään
nolla indeksistä ja seuraavat 30 paikkaa kuuluvat 7 kertaa esiintyneille sanoille. 8 kertaa
esiintyvien sanojen täyttö aloitetaan indeksistä 30 jne.
*/
void fillTopSata(struct node *node, struct node *tulosteArr[], int minValue, int *startIndex) {
    if (node -> vasen != NULL) {
        fillTopSata(node -> vasen, tulosteArr, minValue, startIndex);
    }

    if (node -> oikea != NULL) {
        fillTopSata(node -> oikea, tulosteArr, minValue, startIndex);
    }

    if (node -> count >= minValue) {
        int i = 0;
        while (1) {

            if (tulosteArr[startIndex[node -> count] + i] == NULL) {
                tulosteArr[startIndex[node -> count] + i] = node;
                break;
            }
            i++;
        }
    }
}

// djb2: http://www.cse.yorku.ca/~oz/hash.html
unsigned long hash(char *str) {
    unsigned long hash = 5381;
    int c;

    while (c = *str++) {
        hash = ((hash << 5) + hash) + c;
    }

    return hash;
}
