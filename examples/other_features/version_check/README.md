# Version Checking

There are two options for checking the tool version.

-   `recommended`: Displays a warning when the running version is not this value.  
-   `minimum_required`: Stop reading the json file if the running version is lower than this value.  

> The version string should be of the form `x.x.x`.

![version_error](https://github.com/matyalatte/tuw/assets/69258547/3eb771b5-896f-4503-8383-ed25013de6bb)

```json
{
    "recommended": "1.1.0",
    "minimum_required": "1.0.0",
    "gui": [
        {
            "label": "You can't see this GUI.",
            "command": "echo Hello!",
            "components": []
        }
    ]
}
```
