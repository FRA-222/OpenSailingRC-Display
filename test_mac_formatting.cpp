/*
 * Test de formatage d'adresse MAC pour anémomètre
 * Démontre la conversion d'adresse MAC en string formatée
 */

#include <Arduino.h>

void demonstrateMACFormatting() {
    // Exemple d'adresse MAC d'anémomètre
    uint8_t macAddress[6] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
    
    // Formatage en string avec séparateurs ':'
    String macString = "";
    for (int i = 0; i < 6; i++) {
        if (i > 0) macString += ":";
        if (macAddress[i] < 16) macString += "0";  // Padding pour valeurs < 16
        macString += String(macAddress[i], HEX);
    }
    macString.toUpperCase();
    
    Serial.println("Adresse MAC originale: {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF}");
    Serial.println("ID formaté: " + macString);
    Serial.println("Résultat attendu: AA:BB:CC:DD:EE:FF");
    
    // Test avec une autre adresse
    uint8_t macAddress2[6] = {0x01, 0x02, 0x03, 0x0A, 0x0B, 0x0C};
    String macString2 = "";
    for (int i = 0; i < 6; i++) {
        if (i > 0) macString2 += ":";
        if (macAddress2[i] < 16) macString2 += "0";
        macString2 += String(macAddress2[i], HEX);
    }
    macString2.toUpperCase();
    
    Serial.println("\nDeuxième test:");
    Serial.println("Adresse MAC originale: {0x01, 0x02, 0x03, 0x0A, 0x0B, 0x0C}");
    Serial.println("ID formaté: " + macString2);
    Serial.println("Résultat attendu: 01:02:03:0A:0B:0C");
}

/*
 * Exemple de sortie:
 * 
 * Adresse MAC originale: {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF}
 * ID formaté: AA:BB:CC:DD:EE:FF
 * Résultat attendu: AA:BB:CC:DD:EE:FF
 * 
 * Deuxième test:
 * Adresse MAC originale: {0x01, 0x02, 0x03, 0x0A, 0x0B, 0x0C}
 * ID formaté: 01:02:03:0A:0B:0C
 * Résultat attendu: 01:02:03:0A:0B:0C
 */