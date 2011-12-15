#!/usr/bin/env python
import roslib; roslib.load_manifest('object_recognition_core')
import object_recognition
from object_recognition import dbtools, models
import couchdb
import argparse
import subprocess
import sys
def parse_args():
    parser = argparse.ArgumentParser(description='Search for an objects in the DB based on tags..')
    parser.add_argument('-t', '--tag', metavar='TAG', dest='tag', type=str, default='',
                       help='Tag to search for.')
    parser.add_argument('--object_ids', help='If set, it overrides the list of object_ids in the config file', default=None)
    object_recognition.dbtools.add_db_arguments(parser)
    args = parser.parse_args()
    return args
def meshlab(filename_in, filename_out):
    subprocess.check_call((['meshlabserver', '-i', filename_in, '-o', filename_out]))
def househould_insert(thumbnail, meshfile, obj):
    call = ['rosrun', 'object_recognition_household_object_database_sync', 'insert_model', '-G', meshfile, '-T', thumbnail, '-N', obj.object_name, '-D', obj.description, '-I', str(obj.id)]
    call += list(obj.tags)
    print 'Inserting:\n', ' '.join(call)
    #subprocess.check_call(call)
if __name__ == "__main__":
    args = parse_args()
    couch = couchdb.Server(args.db_root)
    db = dbtools.init_object_databases(couch)
    if vars(args).get('object_ids', None):
        results = []
        object_ids = eval(vars(args).get('object_ids'))
        for obj in models.Object.all(db):
            if obj.id in object_ids:
                results.append(obj)
    else:
        results = models.Object.by_object_name(db)
    import os
    if not os.path.exists('scratch'):
        os.makedirs('scratch')
    for obj in results:
        print "******************************"
        print "Object Name:", obj.object_name
        observation = models.Observation.by_object_id(db, key=obj.id, limit=1).rows[0]
        image = db.get_attachment(observation, 'image')
        png = 'scratch/'+str(obj.id)+'.png'
        mesh_stl = 'scratch/'+str(obj.id)+'.stl'
        mesh_ply = 'scratch/'+str(obj.id)+'.ply'

        with open(png, 'w') as imagef:
            imagef.write(image.read())
        meshes = models.Model.by_object_id_and['mesh'](db, key=obj.id, limit=1).rows
        if len(meshes) == 0:
            sys.stderr.write("No mesh for object %s, skipping!\n" % (str(obj.id)))
            continue
        mesh = db.get_attachment(meshes[0], 'mesh.stl')
        
        with open(mesh_stl, 'w') as meshf:
            meshf.write(mesh.read())
        meshlab(mesh_stl, mesh_ply)
        househould_insert(thumbnail=png, meshfile=mesh_ply, obj=obj)

