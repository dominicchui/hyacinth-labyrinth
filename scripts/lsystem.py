import json
import bpy
import os
import re
import numpy as np

TYPE_DOL = 'DOL'
TYPE_PDOL = 'PDOL'

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

def generate_lsystem_DOL(axiom, rules, iters):
    '''
    Generates the string representation of a DOL (non-parametric,
    deterministic, context-free) L-system.
    '''
    string = axiom
    rules_trans = str.maketrans(rules)
    for i in range(iters):
        string = string.translate(rules)
    return string

def generate_regex(module):
    '''
    Generates a regular expression which can be used to find
    strings which match a given "module", which is just a symbol with
    parameters like A(l,w).
    Written with help from ChatGPT.
    '''
    first_symbol = module[0]
    n = module.count(',') + 1 # num params

    # Match the first symbol and opening paren
    pattern = first_symbol + r'\('

    # Match the first parameter
    # pattern += r'(\d+(?:\.\d+)?)'
    pattern += r'([^),]+)'

    # Match the rest of the comma-separated parameters
    # pattern += r',(\d+(?:\.\d+)?)' * (n-1)
    pattern += r',([^),]+)' * (n-1)

    # Add the closing paren
    pattern += r'\)'

    return pattern

def generate_lsystem_PDOL(constants, axiom, rules, iters):
    '''
    Generates the string representation of a PDOL (parametric,
    deterministic, context-free) L-system.
    '''
    string = axiom
    # On each iteration, we want to find all occurrences of patterns
    # matching a rule's predecessor
    for i in range(iters):
        new_string = ''
        for k in range(len(string)):
            # Determine if the symbol at k is just a single letter
            letter = string[k]
            # If it's just a character by itself...
            if k+1 >= len(string) or string[k+1] != '(':
                match_found = False
                for rule in rules:
                    if rule[0] == letter:
                        # Add its replacement to the string
                        new_string += rule[1]
                        match_found = True
                # If no matching rule was found, append the letter as-is
                if not match_found:
                    new_string += letter
            # Otherwise, if it's a "module", meaning a character with some params...
            else:
                # Extract the entire module
                l = k + 1
                module = letter
                while string[l] != ')':
                    module += string[l]
                    l += 1
                module += ')'
                # Search for a rule which matches this module
                match_found = False
                for rule in rules:
                    # If starting letter doesn't match, it isn't a match
                    if rule[0][0] != letter:
                        continue

                    # If number of commas is not the same, it isn't a match
                    if rule[0].count(',') != module.count(','):
                        continue

                    # Otherwise, it is a match so extract the names/values of each param
                    pred = rule[0]
                    pred_regex = generate_regex(pred)
                    param_vals = list(re.match(pred_regex, module).groups())
                    param_names = list(re.match(pred_regex, pred).groups())
                    print(param_vals)
                    print(param_names)

def generate_lsystem(type, lsystem, iters):
    '''
    Generates the string representation of the L-system.
    '''
    if (type == TYPE_DOL):
        return generate_lsystem_DOL(lsystem['axiom'], 
                                    lsystem['rules'], 
                                    iters)
    else:
        return generate_lsystem_PDOL(lsystem['numerical_constants'], 
                                     lsystem['axiom'], 
                                     lsystem['rules'], 
                                     iters)

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

def create_geometry(type, lstring, step_size, stem_radius, angle_incr, tesselation_level):
    '''
    Creates a list of vertices and faces from the L-system string.
    This is the "geometric interpretation" of the string.
    '''
    if type == TYPE_DOL:
        return create_geometry_DOL(lstring, step_size, stem_radius,
                                   angle_incr, tesselation_level)
    else:
        return None # TODO

def create_geometry_DOL(lstring, step_size, stem_radius, angle_incr, tesselation_level):
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
    # Enter object mode
    bpy.ops.object.mode_set(mode='OBJECT')
    
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

    # Enter Edit Mode
    bpy.context.view_layer.objects.active = obj
    bpy.ops.object.mode_set(mode='EDIT')
    bpy.ops.mesh.select_all(action='SELECT')

    # Triangulate faces
    bpy.ops.mesh.quads_convert_to_tris(quad_method='BEAUTY', ngon_method='BEAUTY')
    
    # Return to object mode
    bpy.ops.object.mode_set(mode='OBJECT')

def parse_json(filename):
    with open(filename, 'r') as json_file:
        settings = json.load(json_file)
    return settings

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

    # Just used for running in Blender
    os.chdir('/Users/echen/Desktop/csci2230/hyacinth-labyrinth/scripts')

    # Choose which lsystem json file we want to use
    filename = 'tree.json'

    # Parse the json to get a dictionary of all settings
    lsystems_dir = os.path.join('lsystems')
    settings = parse_json(os.path.join(lsystems_dir, filename))

    # Generate the L-system string by repeatedly applying rules
    lstring = generate_lsystem(settings['type'], 
                               settings['lsystem'],
                               settings['num_iters'])

    # Apply geometric interpretation
    vertices, faces = create_geometry(settings['type'],
                                      lstring, 
                                      settings['step_size'], 
                                      settings['stem_radius'], 
                                      np.radians(settings['angle_incr']), 
                                      settings['tesselation_level'])

    # Add the geometry to the scene as a mesh
    add_geometry_to_scene(vertices, faces)
