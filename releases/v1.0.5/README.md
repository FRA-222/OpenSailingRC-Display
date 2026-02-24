# OpenSailingRC Display v1.0.5

## Fichiers

| Fichier | Description |
|---------|-------------|
| `OpenSailingRC_Display_v1.0.5_MERGED.bin` | Firmware fusionné pour M5Burner (flash à 0x0) |
| `SHA256SUMS.txt` | Checksums de vérification |
| `RELEASE_NOTES_V1.0.5.md` | Notes de version détaillées |

## Installation rapide

1. Ouvrir **M5Burner**
2. **Custom Firmware** → sélectionner `OpenSailingRC_Display_v1.0.5_MERGED.bin`
3. **Adresse : 0x0** ⚠️
4. **Erase Flash** : coché
5. **Burn**

## Vérification du checksum

```bash
shasum -a 256 OpenSailingRC_Display_v1.0.5_MERGED.bin
```

Résultat attendu : voir `SHA256SUMS.txt`
