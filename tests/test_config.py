import copy
import importlib.util
import json
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('generator', ROOT / 'scripts/generate_config.py')
generator = importlib.util.module_from_spec(spec)
spec.loader.exec_module(generator)


class ConfigTests(unittest.TestCase):
    def setUp(self):
        self.data = json.loads((ROOT / 'config/patterns.json').read_text())

    def test_default_and_empty_tag_map(self):
        generator.validate(self.data)
        self.data['tags'] = []
        self.data['unknown_tag_pattern'] = None
        self.assertIn('return -1;', generator.render(self.data))

    def test_rejects_unsafe_values(self):
        for step in [[501, 10], [10, 231], [0, 1], [True, 10], [10.5, 1]]:
            data = copy.deepcopy(self.data)
            data['patterns'][0]['steps'] = [step]
            with self.assertRaises(ValueError):
                generator.validate(data)

    def test_rejects_continuous_on_and_long_total(self):
        for steps in [[[500, 10]] * 4, [[500, 0]] * 6 + [[1, 10]], [[1, 0]]]:
            data = copy.deepcopy(self.data)
            data['patterns'][0]['steps'] = steps
            with self.assertRaises(ValueError):
                generator.validate(data)

    def test_rejects_duplicate_or_invalid_names(self):
        for name in ['hello', 'x";bad', 'UPPER', 'x' * 25]:
            data = copy.deepcopy(self.data)
            data['patterns'][1]['name'] = name
            with self.assertRaises(ValueError):
                generator.validate(data)

    def test_rejects_bad_uid_and_unknown_mapping(self):
        for tag in [{'uid': 'abc', 'pattern': 'hello'},
                    {'uid': 'DEADBEEF', 'pattern': 'missing'}]:
            data = copy.deepcopy(self.data)
            data['tags'] = [tag]
            with self.assertRaises(ValueError):
                generator.validate(data)

    def test_rejects_too_short_cooldown(self):
        self.data['cooldown_ms'] = 1
        with self.assertRaises(ValueError):
            generator.validate(self.data)
