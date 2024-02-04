import podio

def check_collections(filename, names):
    podio_reader = podio.root_io.Reader(filename)
    for frame in podio_reader.get('events'):
        available = set(frame.collections)
        if available != set(names):
            print(f'These collections should be in the frame but are not: {set(names) - available}')
            print(f'These collections are in the frame but should not be: {available - set(names)}')
            raise RuntimeError('Collections in frame do not match expected collections')

check_collections('functional_transformer.root', ['MCParticles', 'NewMCParticles'])
check_collections('functional_transformer_multiple.root', ['VectorFloat', 'MCParticles1', 'MCParticles2', 'SimTrackerHits', 'TrackerHits', 'Tracks', 'Counter', 'NewMCParticles'])
check_collections('functional_transformer_multiple_output_commands.root', ['VectorFloat', 'MCParticles1', 'MCParticles2', 'SimTrackerHits', 'TrackerHits'])
check_collections('/tmp/a/b/c/output_k4test_exampledata_producer.root', ['MCParticles'])
