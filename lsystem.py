import bpy
import os
import numpy as np

def get_cross_section_vertices(pos, rot, stem_radius, tesselation_level):
    '''
    Returns coords of vertices arranged in a regular polygon around position,
    in a plane whose normal is aligned with heading
    '''

    # Get the heading, left, and up vectors
    heading = rot[:, 0].reshape((3))
    left = rot[:, 1].reshape((3))
    up = rot[:, 2].reshape((3))

    vertices = []

    # Get regularly spaced vertices in the plane p spanned by left and up
    theta_step = 2 * np.pi / tesselation_level
    theta = 0
    for i in range(tesselation_level):
        vertices.append(pos +
                        stem_radius * np.cos(theta) * left +
                        stem_radius * np.sin(theta) * up)
        theta += theta_step

    return [tuple(vertex) for vertex in vertices]

def get_faces(vertices, tesselation_level):
    '''
    Returns faces connecting the most recently added set of vertices to
    the set of vertices added before that.
    '''
    vertex_indices = list(range(len(vertices)))
    curr_vertices = vertex_indices[-tesselation_level:]
    prev_vertices = vertex_indices[-2*tesselation_level:-tesselation_level]
    faces = []
    for i in range(tesselation_level):
        next_i = i + 1
        # Wrap around to first index
        if next_i >= tesselation_level:
            next_i = 0
        faces.append((prev_vertices[i], curr_vertices[i],
                      curr_vertices[next_i], prev_vertices[next_i]))
    return faces

def rewrite(string, rules):
    '''
    Rewrites the given string according to the given list of production rules.
    Returns the rewritten string.
    '''
    string = string.translate(rules)
    return string

def generate_lsystem(axiom, rules, iters):
    '''
    Generates the string representation of the L-system.
    '''
    string = axiom
    rules_trans = str.maketrans(rules)
    for i in range(iters):
        string = rewrite(string, rules_trans)
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
    return rot @ R_L

def roll(rot, alpha):
    R_H = np.array([[1,           0,             0],
                   [0, np.cos(alpha), -np.sin(alpha)],
                   [0, np.sin(alpha), np.cos(alpha)]])
    return rot @ R_H

def create_geometry(lstring, step_size, stem_radius, angle_incr, tesselation_level):
    '''
    Creates a list of vertices and faces from the L-system string.
    This is the "geometric interpretation" of the string.
    '''
    # Track current position
    curr_pos = np.array([0, 0, 0])
    # Use a 3x3 matrix to track our current rotation.
    # Start facing towards +z axis
    curr_rot = np.array([
        [0, 0, 1],
        [0, 1, 0],
        [1, 0, 0],
    ])
    # Track whether or not we are inside of a leaf and vertices of current leaf
    in_leaf = False
    leaf_vertices = []

    # Use a stack (list) to track pushed states to return to.
    states = []

    # Store the vertices and faces generated as we go
    vertices = []
    faces = []

    # Process the string symbol by symbol
    for symbol in lstring:
        if symbol == 'F' or symbol == 'G':
            # Add vertices at the beginning of the segment
            vertices.extend(get_cross_section_vertices(curr_pos,
                                                       curr_rot,
                                                       stem_radius,
                                                       tesselation_level))
            # Move forward
            curr_pos = forward(curr_pos, curr_rot, step_size)
            # Add vertices at the end of the segment
            vertices.extend(get_cross_section_vertices(curr_pos,
                                                       curr_rot,
                                                       stem_radius,
                                                       tesselation_level))
            # Add faces connecting the sets of vertices
            faces.extend(get_faces(vertices, tesselation_level))
        elif symbol == 'f':
            if in_leaf:
                vertices.append(tuple(curr_pos))
                leaf_vertices.append(len(vertices) - 1)
                curr_pos = forward(curr_pos, curr_rot, step_size)
            else:
                curr_pos = forward(curr_pos, curr_rot, step_size)
        elif symbol == '+':
            curr_rot = yaw(curr_rot, angle_incr)
        elif symbol == '-':
            curr_rot = yaw(curr_rot, -angle_incr)
        elif symbol == '&':
            curr_rot = pitch(curr_rot, angle_incr)
        elif symbol == '^' or symbol == '∧':
            curr_rot = pitch(curr_rot, -angle_incr)
        elif symbol == '~':
            curr_rot = roll(curr_rot, angle_incr)
        elif symbol == '/':
            curr_rot = roll(curr_rot, -angle_incr)
        elif symbol == '|':
            curr_rot = yaw(curr_rot, np.radians(180))
        elif symbol == '[':
            states.append((curr_pos, curr_rot))
        elif symbol == ']':
            state = states.pop()
            curr_pos = state[0]
            curr_rot = state[1]
        elif symbol == '{':
            in_leaf = True
        elif symbol == '}':
            faces.append(tuple(leaf_vertices))
            in_leaf = False
            leaf_vertices = []
        else:
            # Symbols with no geometric interpretation are skipped
            continue

    return vertices, faces
    
def add_geometry_to_scene(vertices, faces):
    '''
    Creates a mesh from the list of vertices and list of faces
    generated by create_geometry.
    '''
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
    # (see http://algorithmicbotany.org/papers/abop/abop.pdf):
    # ALPHABET:
    #     'F'   move forward in the direction of heading by step size.
    #     '+'   turn left by angle δ, using rotation matrix RU(δ).
    #     '-'   turn right by angle δ, using rotation matrix RU(−δ).
    #     '&'   pitch down by angle δ, using rotation matrix RL(δ).
    #     '^'   pitch up by angle δ, using rotation matrix RL(−δ).
    #     '~'   roll left by angle δ, using rotation matrix RH(δ). (~ used in place of \)
    #     '/'   roll right by angle δ, using rotation matrix RH(−δ).
    #     '|'   turn around, using rotation matrix RU(180◦).
    #     '['   push current position and orientation to stack.
    #     ']'   set current position and orientation to stack top and pop.
    # NOTE: symbols outside the alphabet may also be used,
    #     but they won't have any geometric interpretation

    # General settings
    NUM_ITERATIONS = 6

    # Geometry settings
    STEP_SIZE = 5
    STEM_RADIUS = 0.5
    ANGLE_INCR = np.radians(90)
    TESSELATION_LEVEL = 3

    # L-system definition
    bush_axiom = "A"
    bush_rules = {"A": "[&FL!A]/////'[&FL!A]///////'[&FL!A]",
                  "F": "S/////F",
                  "S": "FL",
                  "L": "['''∧∧{-f+f+f-|-f+f+f}]"}
                     
    test_axiom = "F"
    test_rules = {"F": "F+G",
                  "G": "F-G"}

    # Generate the L-system string by repeatedly applying rules
    lstring = generate_lsystem(test_axiom, test_rules, NUM_ITERATIONS)

    # Apply geometric interpretation
    vertices, faces = create_geometry(lstring, STEP_SIZE, STEM_RADIUS, ANGLE_INCR, TESSELATION_LEVEL)

    # Add the geometry to the scene as a mesh
    add_geometry_to_scene(vertices, faces)