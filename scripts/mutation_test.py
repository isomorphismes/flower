#!/usr/bin/env python3
"""Require semantic rejection, not compilation failure, of gate/architecture regressions."""
import os
from pathlib import Path
import subprocess
import tempfile

root = Path(__file__).resolve().parents[1]
source = (root / 'src/flower.c').read_text()
mutations = {
    'release ignored': ('if (!down) { flower_hold_release(hold,pointer); return false; }', '(void)down;'),
    'pointer identity ignored': (' || pointer!=hold->owner', ''),
    'focus loss ignored': ('if (!focused || !isfinite(frame_seconds)', 'if (!isfinite(frame_seconds)'),
    'simulation disconnected': ('simulation_step(flower,hit,policy);', '(void)flower;'),
}
with tempfile.TemporaryDirectory(prefix='flower-mutants-') as temporary:
    temporary = Path(temporary)
    for name, (before, after) in mutations.items():
        if source.count(before) != 1:
            raise SystemExit(f'Mutation anchor changed: {name}')
        mutant = temporary / 'flower.c'
        mutant.write_text(source.replace(before, after))
        executable = temporary / 'test'
        subprocess.run([os.environ.get('CC', 'cc'), '-std=c11', '-O2', '-DNDEBUG',
                        '-I', str(root / 'src'), str(mutant), str(root / 'src/flower_mesh.c'), str(root / 'tests/test_flower.c'),
                        '-lm', '-o', str(executable)], check=True)
        result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=20)
        if result.returncode != 1 or 'FAIL line' not in result.stderr:
            raise SystemExit(f'Mutant did not fail a runtime check: {name}: {result.returncode}')
        print(f'REJECTED {name}: {result.stderr.strip()}')

    lua_source = Path(os.environ['FLOWER_LUA_SOURCE'])
    lua_object = temporary / 'lua.o'
    subprocess.run([os.environ.get('CC', 'cc'), '-std=c11', '-O2', '-DMAKE_LIB',
                    '-c', str(lua_source / 'onelua.c'), '-o', str(lua_object)], check=True)
    architecture_mutations = {
        'Euclidean folded-nearness brush': (
            'grow_intrinsic_geometry(&flower->net,&flower->skin,distance,policy);',
            'for (int index=0;index<flower->net.node_count;index++) { '
            'FlowerPoint delta=sub(flower->skin.position[index],flower->skin.position[hit.node]); '
            'distance[index]=dot(delta,delta)<0.04f ? 0 : -1; } '
            'grow_intrinsic_geometry(&flower->net,&flower->skin,distance,policy);'),
        'render diagonal admitted as material adjacency': (
            'net->neighbor[id][net->degree[id]++]=(uint16_t)vertex(net,ring+1,slice);',
            'net->neighbor[id][net->degree[id]++]=(uint16_t)vertex(net,ring+1,slice+1);'),
        'Lua policy bypasses native hold gate': (
            'if (!hold->captured || hold->blocked || pointer!=hold->owner) return false;',
            'if ((!hold->captured || hold->blocked || pointer!=hold->owner) && policy->rate<=0) return false;'),
        'repeated graph paths multiply growth': (
            'skin->growth[index]+gain)', 'skin->growth[index]+gain*(distance[index]>1?2:1))'),
    }
    for name, (before, after) in architecture_mutations.items():
        if source.count(before) != 1:
            raise SystemExit(f'Mutation anchor changed: {name}: {source.count(before)}')
        mutant = temporary / 'flower.c'
        mutant.write_text(source.replace(before, after))
        executable = temporary / 'test-architecture'
        subprocess.run([os.environ.get('CC', 'cc'), '-std=c11', '-O2', '-DNDEBUG',
                        '-I', str(root / 'src'), '-I', str(lua_source), str(mutant),
                        str(root / 'src/flower_mesh.c'), str(root / 'src/flower_policy.c'),
                        str(root / 'src/flower_camera.c'),
                        str(root / 'tests/test_architecture.c'), str(lua_object), '-lm', '-o', str(executable)], check=True)
        result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=20)
        if result.returncode != 1 or 'FAIL line' not in result.stderr:
            raise SystemExit(f'Mutant did not fail a runtime check: {name}: {result.returncode}')
        print(f'REJECTED {name}: {result.stderr.strip()}')
