import bpy
import os
import numpy as np

def get_cross_section_vertices(position, orientation, tesselation_level):
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
    heading = rot[:, 0].reshape((3))
    new_pos = pos + heading * step_size
    return new_pos

def yaw(rot, alpha):
    R_U = np.array([[np.cos(alpha), np.sin(alpha), 0],
                   [-np.sin(alpha), np.cos(alpha), 0],
                   [0,           0,             1]])
    return rot @ R_U

def pitch(rot, alpha):
    R_L = np.array([[np.cos(alpha), 0, -np.sin(alpha)],
                   [0,           1,             0],
                   [np.sin(alpha), 0, np.cos(alpha)]])
    return rot * R_L

def roll(rot, alpha):
    R_H = np.array([[1,           0,             0],
                   [0, np.cos(alpha), -np.sin(alpha)],
                   [0, np.sin(alpha), np.cos(alpha)]])
    return rot * R_H

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
                                                       curr_rot,
                                                       tesselation_level))
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

    return vertices, faces
    
def add_geometry_to_scene(vertices, faces):
    # Clear existing mesh objects
    bpy.ops.object.select_all(action='SELECT')
    bpy.ops.object.delete()

    # Create mesh data
    mesh = bpy.data.meshes.new('MyMesh')
    obj = bpy.data.objects.new('MyObject', mesh)

    # Link object to the scene
    scene = bpy.context.scene
    scene.collection.objects.link(obj) 

    # Set vertices to the mesh
    # Vertices, edges, faces
    mesh.from_pydata(vertices, [], faces)

    # Update mesh geometry
    mesh.update()

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
    NUM_ITERATIONS = 3

    # Geometry settings
    STEP_SIZE = 5
    ANGLE_INCR = np.radians(90)
    TESSELATION_LEVEL = 1

    # L-system definition
    axiom = 'F-F-F-F'
    rules = [('F', 'F-F+F+FF-F-F+F')]
    
    # Validate settings
    assert(all(symbol in ALPHABET for symbol in axiom))
    for rule in rules:
        assert(all(symbol in ALPHABET for symbol in rule[0]))
        assert(all(symbol in ALPHABET for symbol in rule[1]))

    # Generate the L-system string by repeatedly applying rules
    lstring = generate_lsystem(axiom, rules, NUM_ITERATIONS)

    # Apply geometric interpretation
    vertices, faces = create_geometry(lstring, STEP_SIZE, ANGLE_INCR, TESSELATION_LEVEL)

    # Add the geometry to the scene as a mesh
    add_geometry_to_scene(vertices, faces)