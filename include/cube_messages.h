typedef struct MessageLetter {
  char letter;
  // char secret;
} MessageLetter;

#define NFCID_LENGTH 8

typedef struct MessageNfcId {
  char id[NFCID_LENGTH*2 + 1];
} MessageNfcId;
