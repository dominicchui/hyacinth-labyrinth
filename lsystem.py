bpy = None
import os
import numpy as np

def get_cross_section_vertices(position, orientation, num_vertices):
    '''
    Returns coords of vertices arranged in a regular polygon around position,
    in a plane whose normal is aligned with orientation
    '''
    return tuple(position)

def rewrite(string, rules):
    '''
    Rewrites the given string according to the given list of production rules.
    Returns the rewritten string.
    '''
    for rule in rules:
        string = string.replace(rule[0], rule[1])
    return string

def generate_lsystem(axiom, rules, iters):
    string = axiom
    for i in range(iters):
        string = rewrite(string, rules)
    return string

def forward(pos, rot, step_size):
    heading = rot[:, 0]

    return None

def yaw(rot, angle_incr):
    return None

def pitch(rot, angle_incr):
    return None

def roll(rot, angle_incr):
    return None

def create_geometry(lstring, step_size, angle_incr, tesselation_level):
    # Track current position
    curr_pos = np.array([0, 0, 0])
    # Use a 3x3 matrix to track our current rotation.
    curr_rot = np.array([
        [1, 0, 0],
        [0, 1, 0],
        [0, 0, 1],
    ])
    # Store the vertices and faces generated as we go
    vertices = []
    faces = []

    # Process the string symbol by symbol
    for symbol in lstring:
        if symbol == 'F':
            curr_pos = forward(curr_pos, curr_rot, step_size)
            # Add vertices at the end of the segment
            vertices.append(get_cross_section_vertices(curr_pos,
                                                       curr_rot))
            # Add faces to connect to previous cross-section
            # TODO
        elif symbol == '+':
            curr_rot = yaw(curr_rot, angle_incr)
        elif symbol == '-':
            curr_rot = yaw(curr_rot, -angle_incr)
        elif symbol == '&':
            curr_rot = pitch(curr_rot, angle_incr)
        elif symbol == '^':
            curr_rot = pitch(curr_rot, -angle_incr)
        elif symbol == '~':
            curr_rot = roll(curr_rot, angle_incr)
        elif symbol == '/':
            curr_rot = roll(curr_rot, -angle_incr)
        elif symbol == '|':
            curr_rot = yaw(curr_rot, np.radians(180))
        else:
            print('Encountered unknown symbol: {}'.format(symbol))
            continue

if __name__ == '__main__':
    # Constants

    # From ABOP (http://algorithmicbotany.org/papers/abop/abop.pdf):
    #   + Turn left by angle δ, using rotation matrix RU(δ).
    #   − Turn right by angle δ, using rotation matrix RU(−δ).
    #   & Pitch down by angle δ, using rotation matrix RL(δ).
    #   ∧ Pitch up by angle δ, using rotation matrix RL(−δ).
    #   \ Roll left by angle δ, using rotation matrix RH(δ).
    #       NOTE: ~ used in place of \ to avoid escape char issues
    #   / Roll right by angle δ, using rotation matrix RH(−δ).
    #   | Turn around, using rotation matrix RU(180◦).
    ALPHABET = {
        'F',
        '+',
        '-',
        '&',
        '^',
        '~',
        '/',
        '|'
    }

    # General settings
    NUM_ITERATIONS = 4

    # Geometry settings
    STEP_SIZE = 5
    ANGLE_INCR = np.radians(30)
    TESSELATION_LEVEL = 1

    # L-system definition
    axiom = 'F'
    rules = [('F', 'F+G'),
             ('G', 'F-G')]
    
    # Validate settings
    assert(all(symbol in ALPHABET for symbol in axiom))
    for rule in rules:
        assert(all(symbol in ALPHABET for symbol in rule[0]))
        assert(all(symbol in ALPHABET for symbol in rule[1]))

    # Generate the L-system string by repeatedly applying rules
    lstring = generate_lsystem(axiom, rules, NUM_ITERATIONS)

    # Apply geometric interpretation
    create_geometry(lstring, STEP_SIZE, ANGLE_INCR, TESSELATION_LEVEL)