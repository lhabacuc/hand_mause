import unittest
import os
import tempfile
from utils import dist
from config import load_config


class TestUtils(unittest.TestCase):
    def test_dist(self):
        """Test distance calculation between two points"""
        # Test distance between (0,0) and (3,4) should be 5
        result = dist((0, 0), (3, 4))
        self.assertAlmostEqual(result, 5.0, places=5)
        
        # Test distance between same points should be 0
        result = dist((1, 1), (1, 1))
        self.assertAlmostEqual(result, 0.0, places=5)
        
        # Test distance with floats
        result = dist((1.5, 2.5), (1.5, 2.5))
        self.assertAlmostEqual(result, 0.0, places=5)


class TestConfig(unittest.TestCase):
    def test_load_config_creates_default(self):
        """Test that load_config creates a default config if file doesn't exist"""
        with tempfile.TemporaryDirectory() as tmpdir:
            config_path = os.path.join(tmpdir, 'test_config.ini')
            
            # Load config (should create default)
            config = load_config(config_path)
            
            # Verify file was created
            self.assertTrue(os.path.exists(config_path))
            
            # Verify default values
            self.assertEqual(config['pinch_threshold'], '0.04')
            self.assertEqual(config['click_max_duration'], '0.7')
            self.assertEqual(config['drag_min_duration'], '0.8')
            self.assertEqual(config['smoothness'], '0.5')
            self.assertEqual(config['camera_width'], '640')
            self.assertEqual(config['camera_height'], '480')
            self.assertEqual(config['speed_multiplier'], '1.5')

    def test_load_config_reads_existing(self):
        """Test that load_config reads an existing config file"""
        with tempfile.TemporaryDirectory() as tmpdir:
            config_path = os.path.join(tmpdir, 'test_config.ini')
            
            # Create a config file with custom values
            with open(config_path, 'w') as f:
                f.write('[HAND_MOUSE]\n')
                f.write('pinch_threshold = 0.05\n')
                f.write('click_max_duration = 0.8\n')
                f.write('drag_min_duration = 0.9\n')
                f.write('smoothness = 0.6\n')
                f.write('camera_width = 1280\n')
                f.write('camera_height = 720\n')
                f.write('speed_multiplier = 2.0\n')
            
            # Load config
            config = load_config(config_path)
            
            # Verify custom values were loaded
            self.assertEqual(config['pinch_threshold'], '0.05')
            self.assertEqual(config['click_max_duration'], '0.8')
            self.assertEqual(config['drag_min_duration'], '0.9')
            self.assertEqual(config['smoothness'], '0.6')
            self.assertEqual(config['camera_width'], '1280')
            self.assertEqual(config['camera_height'], '720')
            self.assertEqual(config['speed_multiplier'], '2.0')


if __name__ == '__main__':
    unittest.main()
