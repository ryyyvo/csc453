import sys
import random

MAX_CYLINDER = 4999
MIN_CYLINDER = 0
NUM_CYLINDERS = 5000

def read_access_sequence(filename):
    with open(filename, 'r') as f:
        sequence = [int(line.strip()) for line in f if line.strip()]
    return sequence

def generate_random_sequence():
    return [random.randint(MIN_CYLINDER, MAX_CYLINDER) for _ in range(100)]

def fcfs(init_position, sequence):
    total_distance = 0
    curr_position = abs(init_position)

    for track in sequence:
        total_distance += abs(track - curr_position)
        curr_position = track

    return total_distance

def sstf(init_position, sequence):
    total_distance = 0
    curr_position = abs(init_position)
    remaining = sequence.copy()
    
    while remaining:
        closest_track = min(remaining, key=lambda x: abs(x - curr_position))
        total_distance += abs(closest_track - curr_position)
        curr_position = closest_track
        remaining.remove(closest_track)

    return total_distance

def scan(init_position, sequence):
    total_distance = 0
    curr_position = abs(init_position)
    if init_position >= 0:
        direction = 1
    else:
        direction = -1
    
    requests = sorted(sequence)

    if direction == 1:
        right = [track for track in requests if track >= curr_position]
        left = [track for track in requests if track < curr_position]
        
        for track in right:
            total_distance += abs(track - curr_position)
            curr_position = track

        if right and curr_position < MAX_CYLINDER:
            total_distance += MAX_CYLINDER - curr_position
            curr_position = MAX_CYLINDER

        for track in reversed(left):
            total_distance += abs(curr_position - track)
            curr_position = track

    else:
        left = [track for track in requests if track <= curr_position]
        right = [track for track in requests if track > curr_position]
        
        for track in reversed(left):
            total_distance += abs(curr_position - track)
            curr_position = track

        if left and curr_position > MIN_CYLINDER:
            total_distance += curr_position - MIN_CYLINDER
            curr_position = MIN_CYLINDER

        for track in right:
            total_distance += abs(track - curr_position)
            curr_position = track

    return total_distance

def c_scan(init_position, sequence):
    total_distance = 0
    curr_position = abs(init_position)
    if init_position >= 0:
        direction = 1
    else:
        direction = -1
    
    requests = sorted(sequence)

    if direction == 1:
        right = [track for track in requests if track >= curr_position]
        left = [track for track in requests if track < curr_position]
        
        for track in right:
            total_distance += abs(track - curr_position)
            curr_position = track

        if right and left:
            total_distance += MAX_CYLINDER - curr_position
            curr_position = MAX_CYLINDER
            total_distance += MAX_CYLINDER - MIN_CYLINDER
            curr_position = MIN_CYLINDER

        for track in left:
            total_distance += abs(track - curr_position)
            curr_position = track

    else:
        left = [track for track in requests if track <= curr_position]
        right = [track for track in requests if track > curr_position]
        
        for track in reversed(left):
            total_distance += abs(curr_position - track)
            curr_position = track

        if left and right:
            total_distance += curr_position - MIN_CYLINDER
            curr_position = MIN_CYLINDER
            total_distance += MAX_CYLINDER - MIN_CYLINDER
            curr_position = MAX_CYLINDER

        for track in reversed(right):
            total_distance += abs(curr_position - track)
            curr_position = track

    return total_distance
    
def look(init_position, sequence):
    total_distance = 0
    curr_position = abs(init_position)
    if init_position >= 0:
        direction = 1
    else:
        direction = -1
    
    requests = sorted(sequence)

    if direction == 1:  
        right = [r for r in requests if r >= curr_position]
        left = [r for r in requests if r < curr_position]

        for track in right:
            total_distance += abs(track - curr_position)
            curr_position = track

        for track in reversed(left):
            total_distance += abs(curr_position - track)
            curr_position = track
    else: 
        left = [r for r in requests if r <= curr_position]
        right = [r for r in requests if r > curr_position]

        for track in reversed(left):
            total_distance += abs(curr_position - track)
            curr_position = track

        for track in right:
            total_distance += abs(track - curr_position)
            curr_position = track

    return total_distance

def c_look(init_position, sequence):
    total_distance = 0
    curr_position = abs(init_position)
    if init_position >= 0:
        direction = 1
    else:
        direction = -1
    
    requests = sorted(sequence)

    if direction == 1:
        right = [r for r in requests if r >= curr_position]
        left = [r for r in requests if r < curr_position]

        for track in right:
            total_distance += abs(track - curr_position)
            curr_position = track

        if left:
            leftmost = left[0]
            total_distance += abs(curr_position - leftmost)
            curr_position = leftmost
            
            for track in left[1:]:
                total_distance += abs(track - curr_position)
                curr_position = track
    else:
        left = [r for r in requests if r <= curr_position]
        right = [r for r in requests if r > curr_position]

        for track in reversed(left):
            total_distance += abs(curr_position - track)
            curr_position = track

        if right:
            rightmost = right[-1]
            total_distance += abs(rightmost - curr_position)
            curr_position = rightmost
            
            for track in reversed(right[:-1]):
                total_distance += abs(curr_position - track)
                curr_position = track
    
    return total_distance

def main():
    if len(sys.argv) < 2 or len(sys.argv) > 3:
        print("Usage: diskSim INITIAL_POSITION [ACCESS_SEQUENCE_FILE]")
        sys.exit(1)
    
    init_position = int(sys.argv[1])

    if abs(init_position) > MAX_CYLINDER:
        print(f"Error: INITIAL_POSITION must be between -{MAX_CYLINDER} and {MAX_CYLINDER}")
        sys.exit(1)

    if len(sys.argv) == 3:
        sequence = read_access_sequence(sys.argv[2])
    else:
        sequence = generate_random_sequence()

    for track in sequence:
        if track < MIN_CYLINDER or track > MAX_CYLINDER:
            print(f"Error: All track numbers must be between {MIN_CYLINDER} and {MAX_CYLINDER}")
            sys.exit(1)

    print(f"FCFS {fcfs(init_position, sequence)}")
    print(f"SSTF {sstf(init_position, sequence)}")
    print(f"SCAN {scan(init_position, sequence)}")
    print(f"C-SCAN {c_scan(init_position, sequence)}")
    print(f"LOOK {look(init_position, sequence)}")
    print(f"C-LOOK {c_look(init_position, sequence)}")

if __name__ == "__main__":
    main()