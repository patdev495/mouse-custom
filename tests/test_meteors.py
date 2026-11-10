"""Meteor trail tests at the same user-approved UI/config/Windows seams."""
import unittest
import time
import test_desktop as d

class MeteorTests(unittest.TestCase):
    setUp = d.DesktopTests.setUp
    launch = d.DesktopTests.launch
    stop = d.DesktopTests.stop

    def test_meteor_toggle_is_independent_and_persistent(self):
        overlay = self.launch('--settings')
        settings = d.settings_window()
        meteors = d.button(settings, 'Meteor trail')
        self.assertEqual(d.u.SendMessageW(meteors, 0x00F0, 0, 0), 0)
        d.click(meteors)
        self.assertTrue(d.u.IsWindowVisible(overlay))
        self.assertEqual(d.u.SendMessageW(d.button(settings, 'Glow'), 0x00F0, 0, 0), 0)
        d.click(d.button(settings, 'Close'))
        self.stop()
        self.launch('--settings')
        settings = d.settings_window()
        self.assertEqual(d.u.SendMessageW(d.button(settings, 'Meteor trail'), 0x00F0, 0, 0), 1)

    def test_meteor_options_are_independent_and_survive_restart(self):
        self.launch('--settings')
        settings = d.settings_window()
        d.click(d.button(settings, 'Options...'))
        options = d.wait_for(lambda: d.window('Meteor Options'), 'Meteor options did not open')
        names = ('Density', 'Star size', 'Fall speed', 'Opacity (%)')
        sliders = [d.wait_for(lambda name=name: d.u.FindWindowExW(options, None, 'msctls_trackbar32', name),
                              'Missing meteor control: ' + name) for name in names]
        for slider in sliders:
            d.u.SendMessageW(slider, 0x0100, 0x23, 0)
        color = d.C.create_unicode_buffer('FF3300')
        edit = d.u.FindWindowExW(options, None, 'Edit', None)
        d.u.SendMessageW(edit, 0x000C, 0, d.C.cast(color, d.C.c_void_p).value)
        d.click(d.button(options, 'Close'))
        d.click(d.button(settings, 'Close'))
        self.stop()
        self.launch('--settings')
        settings = d.settings_window()
        self.assertEqual(d.text(d.u.FindWindowExW(settings, None, 'Edit', None)), 'FFD200')
        d.click(d.button(settings, 'Options...'))
        options = d.wait_for(lambda: d.window('Meteor Options'), 'Options did not reopen')
        actual = tuple(d.u.SendMessageW(d.u.FindWindowExW(options, None, 'msctls_trackbar32', name),
                                        0x0400, 0, 0) for name in names)
        self.assertEqual(actual, (100, 8, 240, 100))
        self.assertEqual(d.text(d.u.FindWindowExW(options, None, 'Edit', None)), 'FF3300')

    def test_stars_fall_after_movement_then_fade_without_hiding_highlight(self):
        (self.folder / 'mouse-options.ini').write_text('[highlight]\nmeteor=1\n', encoding='utf-16')
        overlay = self.launch()
        d.replay_cursor(self.process, self.exe, 400, 260)
        time.sleep(1.15)  # Let any setup movement expire.
        foreground = d.u.GetForegroundWindow()
        d.replay_cursor(self.process, self.exe, 480, 260)
        stars = d.wait_for(lambda: (h if (h := d.window('Mouse Options Meteors')) and
                                   d.u.IsWindowVisible(h) else None), 'No stars appeared on the pointer path')
        first = d.bounds(stars)
        time.sleep(.16)
        self.assertGreater(d.bounds(stars)[1], first[1] + 4, 'Stars must fall after the pointer stops')
        self.assertEqual(d.u.GetForegroundWindow(), foreground)
        self.assertTrue(d.u.IsWindowVisible(overlay))
        d.wait_for(lambda: not d.window('Mouse Options Meteors'), 'Stars must fade and release their window', seconds=2)
        self.assertTrue(d.u.IsWindowVisible(overlay))

if __name__ == '__main__':
    unittest.main(verbosity=2)
