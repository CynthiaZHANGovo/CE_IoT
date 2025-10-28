| Test Case                  | Input / Action                   | Expected Behavior                                     | Result |
| -------------------------- | -------------------------------- | ----------------------------------------------------- | ------ |
| Distance change (approach) | Move hand from 1m → 10cm         | Color shifts smoothly from blue → red; scrolls upward | ✅      |
| Distance change (withdraw) | Move hand from 10cm → 1m         | Color shifts from red → blue; scrolls downward        | ✅      |
| Quick motion               | Move hand rapidly back and forth | Brightness increases with motion speed                | ✅      |
| Quick motion               | Move hand rapidly back and forth but for a small distance | Brightness increases with motion speed (but not that stable)      | ?      |
| No motion                  | Keep still for 5s                | Light remains stable (no flicker)                     | ✅      |
| MQTT communication         | Broker online                    | Messages appear correctly on MQTT Explorer            | ✅      |
