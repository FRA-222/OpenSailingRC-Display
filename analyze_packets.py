#!/usr/bin/env python3
"""
Script d'analyse de perte de paquets et doublons
Compare les fichiers JSON du BoatGPS (émetteur) et du Display (récepteur)
"""

import json
import sys
from collections import Counter

def analyze_file(filepath, source_name):
    """Analyse un fichier JSON et retourne les statistiques"""
    sequences = []
    
    try:
        with open(filepath, 'r') as f:
            for line_num, line in enumerate(f, 1):
                line = line.strip()
                if not line:
                    continue
                    
                try:
                    data = json.loads(line)
                    
                    # Extraire sequenceNumber selon la structure
                    if 'boat' in data and 'sequenceNumber' in data['boat']:
                        seq = data['boat']['sequenceNumber']
                    elif 'sequenceNumber' in data:
                        seq = data['sequenceNumber']
                    else:
                        print(f"⚠️  Ligne {line_num}: Pas de sequenceNumber trouvé")
                        continue
                    
                    sequences.append(seq)
                    
                except json.JSONDecodeError as e:
                    print(f"❌ Erreur JSON ligne {line_num}: {e}")
                    continue
                    
    except FileNotFoundError:
        print(f"❌ Fichier non trouvé: {filepath}")
        return None
    
    if not sequences:
        print(f"❌ Aucune séquence trouvée dans {filepath}")
        return None
    
    # Statistiques
    total_packets = len(sequences)
    unique_sequences = len(set(sequences))
    duplicates = total_packets - unique_sequences
    
    # Comptage des doublons par numéro
    counter = Counter(sequences)
    duplicate_numbers = {seq: count for seq, count in counter.items() if count > 1}
    
    # Plage et pertes
    min_seq = min(sequences)
    max_seq = max(sequences)
    expected_packets = max_seq - min_seq + 1
    lost_packets = expected_packets - unique_sequences
    
    # Taux
    loss_rate = (lost_packets / expected_packets * 100) if expected_packets > 0 else 0
    duplicate_rate = (duplicates / total_packets * 100) if total_packets > 0 else 0
    
    print(f"\n{'='*60}")
    print(f"📊 ANALYSE: {source_name}")
    print(f"{'='*60}")
    print(f"Fichier: {filepath}")
    print(f"\n📦 PAQUETS:")
    print(f"  Total de lignes:       {total_packets}")
    print(f"  Séquences uniques:     {unique_sequences}")
    print(f"  Doublons:              {duplicates} ({duplicate_rate:.1f}%)")
    
    print(f"\n📈 PLAGE:")
    print(f"  Première séquence:     #{min_seq}")
    print(f"  Dernière séquence:     #{max_seq}")
    print(f"  Étendue:               {expected_packets} paquets attendus")
    
    print(f"\n❌ PERTES:")
    print(f"  Paquets perdus:        {lost_packets}")
    print(f"  Taux de perte:         {loss_rate:.1f}%")
    
    if duplicate_numbers:
        print(f"\n🔄 DOUBLONS DÉTAILLÉS:")
        # Trier par nombre d'occurrences (plus fréquent en premier)
        sorted_dups = sorted(duplicate_numbers.items(), key=lambda x: x[1], reverse=True)
        
        # Afficher les 10 premiers
        for seq, count in sorted_dups[:10]:
            print(f"  Séquence #{seq}: {count} fois")
        
        if len(sorted_dups) > 10:
            print(f"  ... et {len(sorted_dups) - 10} autres numéros en doublon")
    
    # Trouver les trous dans la séquence
    all_sequences = set(sequences)
    missing = []
    for i in range(min_seq, max_seq + 1):
        if i not in all_sequences:
            missing.append(i)
    
    if missing and len(missing) <= 50:
        print(f"\n🕳️  SÉQUENCES MANQUANTES:")
        # Grouper les séquences consécutives
        groups = []
        start = missing[0]
        end = missing[0]
        
        for i in range(1, len(missing)):
            if missing[i] == end + 1:
                end = missing[i]
            else:
                if start == end:
                    groups.append(f"#{start}")
                else:
                    groups.append(f"#{start}-#{end}")
                start = end = missing[i]
        
        # Dernière groupe
        if start == end:
            groups.append(f"#{start}")
        else:
            groups.append(f"#{start}-#{end}")
        
        # Afficher par lignes de 10
        for i in range(0, len(groups), 10):
            print(f"  {', '.join(groups[i:i+10])}")
    elif missing:
        print(f"\n🕳️  SÉQUENCES MANQUANTES: {len(missing)} (trop nombreuses pour afficher)")
    
    return {
        'sequences': sequences,
        'unique_sequences': set(sequences),
        'min': min_seq,
        'max': max_seq,
        'total': total_packets,
        'unique': unique_sequences,
        'duplicates': duplicates,
        'lost': lost_packets,
        'loss_rate': loss_rate,
        'duplicate_rate': duplicate_rate
    }

def compare_files(boat_file, display_file):
    """Compare les fichiers du bateau et du display"""
    print("\n" + "="*60)
    print("🔍 COMPARAISON BATEAU vs DISPLAY")
    print("="*60)
    
    boat_stats = analyze_file(boat_file, "BATEAU (Émetteur)")
    display_stats = analyze_file(display_file, "DISPLAY (Récepteur)")
    
    if not boat_stats or not display_stats:
        return
    
    # Séquences communes et uniques
    boat_seqs = boat_stats['unique_sequences']
    display_seqs = display_stats['unique_sequences']
    
    common = boat_seqs & display_seqs
    only_boat = boat_seqs - display_seqs
    only_display = display_seqs - boat_seqs
    
    print(f"\n🔗 CORRESPONDANCE:")
    print(f"  Séquences émises (bateau):   {len(boat_seqs)}")
    print(f"  Séquences reçues (display):  {len(display_seqs)}")
    print(f"  Correspondances:             {len(common)}")
    print(f"  Pertes en transmission:      {len(only_boat)} ({len(only_boat)/len(boat_seqs)*100:.1f}%)")
    
    if only_display:
        print(f"  ⚠️  Reçues mais non émises:    {len(only_display)} (anormal!)")
    
    print(f"\n📡 EFFICACITÉ DE TRANSMISSION:")
    efficiency = len(common) / len(boat_seqs) * 100 if boat_seqs else 0
    print(f"  Taux de réception:           {efficiency:.1f}%")
    
    # Analyser les doublons du Display
    if display_stats['duplicates'] > 0:
        print(f"\n🔄 DOUBLONS SUR LE DISPLAY:")
        print(f"  Total de doublons:           {display_stats['duplicates']}")
        print(f"  Cause probable:              Callbacks ESP-NOW multiples")
        print(f"  ✅ Solution:                  Filtre lastStoredSequence implémenté")

if __name__ == "__main__":
    if len(sys.argv) == 1:
        print("Usage:")
        print(f"  {sys.argv[0]} <fichier_display.json>")
        print(f"  {sys.argv[0]} <fichier_bateau.json> <fichier_display.json>")
        print()
        print("Exemples:")
        print(f"  {sys.argv[0]} /Volumes/DISPLAY/replay/2025-11-23_04-33-59.json")
        print(f"  {sys.argv[0]} /Volumes/BOATGPS/2025-11-23_04-33-59.json /Volumes/DISPLAY/replay/2025-11-23_04-33-59.json")
        sys.exit(1)
    
    if len(sys.argv) == 2:
        analyze_file(sys.argv[1], "FICHIER")
    else:
        compare_files(sys.argv[1], sys.argv[2])
