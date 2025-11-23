#!/usr/bin/env python3
"""
Script d'analyse multi-bateaux
Analyse les pertes et doublons par bateau (basé sur MAC address)
"""

import json
import sys
from collections import defaultdict, Counter

def analyze_multiboat_file(filepath):
    """Analyse un fichier JSON avec plusieurs bateaux"""
    boats_data = defaultdict(list)  # {mac: [sequences]}
    
    try:
        with open(filepath, 'r') as f:
            for line_num, line in enumerate(f, 1):
                line = line.strip()
                if not line:
                    continue
                    
                try:
                    data = json.loads(line)
                    
                    # Extraire MAC et sequenceNumber
                    if 'boat' in data:
                        boat = data['boat']
                        mac = boat.get('name', 'UNKNOWN')  # MAC address en string
                        seq = boat.get('sequenceNumber')
                        
                        if seq is not None:
                            boats_data[mac].append(seq)
                    
                except json.JSONDecodeError as e:
                    print(f"❌ Erreur JSON ligne {line_num}: {e}")
                    continue
                    
    except FileNotFoundError:
        print(f"❌ Fichier non trouvé: {filepath}")
        return None
    
    if not boats_data:
        print(f"❌ Aucune donnée bateau trouvée dans {filepath}")
        return None
    
    print(f"\n{'='*70}")
    print(f"📊 ANALYSE MULTI-BATEAUX")
    print(f"{'='*70}")
    print(f"Fichier: {filepath}")
    print(f"Nombre de bateaux détectés: {len(boats_data)}")
    print()
    
    # Analyser chaque bateau
    for boat_num, (mac, sequences) in enumerate(sorted(boats_data.items()), 1):
        print(f"{'='*70}")
        print(f"🚤 BATEAU #{boat_num}: {mac}")
        print(f"{'='*70}")
        
        if not sequences:
            print("⚠️  Aucune séquence pour ce bateau")
            continue
        
        # Statistiques
        total_packets = len(sequences)
        unique_sequences = len(set(sequences))
        duplicates = total_packets - unique_sequences
        
        # Comptage des doublons
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
            sorted_dups = sorted(duplicate_numbers.items(), key=lambda x: x[1], reverse=True)
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
        
        if missing and len(missing) <= 30:
            print(f"\n🕳️  SÉQUENCES MANQUANTES:")
            groups = []
            if missing:
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
                
                if start == end:
                    groups.append(f"#{start}")
                else:
                    groups.append(f"#{start}-#{end}")
                
                for i in range(0, len(groups), 10):
                    print(f"  {', '.join(groups[i:i+10])}")
        elif missing:
            print(f"\n🕳️  SÉQUENCES MANQUANTES: {len(missing)} (trop nombreuses pour afficher)")
        
        print()
    
    # Résumé global
    print(f"{'='*70}")
    print(f"📊 RÉSUMÉ GLOBAL")
    print(f"{'='*70}")
    
    total_all_packets = sum(len(seqs) for seqs in boats_data.values())
    total_all_unique = sum(len(set(seqs)) for seqs in boats_data.values())
    total_all_duplicates = total_all_packets - total_all_unique
    
    print(f"Total paquets reçus (tous bateaux): {total_all_packets}")
    print(f"Total séquences uniques:            {total_all_unique}")
    print(f"Total doublons:                     {total_all_duplicates}")
    
    # Calculer taux de perte moyen
    all_losses = []
    for sequences in boats_data.values():
        if sequences:
            min_s = min(sequences)
            max_s = max(sequences)
            expected = max_s - min_s + 1
            unique = len(set(sequences))
            lost = expected - unique
            if expected > 0:
                loss_rate = 100.0 * lost / expected
                all_losses.append(loss_rate)
    
    if all_losses:
        avg_loss = sum(all_losses) / len(all_losses)
        print(f"Taux de perte moyen:                {avg_loss:.1f}%")
    
    print()

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage:")
        print(f"  {sys.argv[0]} <fichier_display.json>")
        print()
        print("Exemples:")
        print(f"  {sys.argv[0]} /Volumes/DISPLAY/replay/2025-11-23_05-25-09.json")
        sys.exit(1)
    
    analyze_multiboat_file(sys.argv[1])
