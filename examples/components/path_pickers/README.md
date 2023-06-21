# Path Pickers

You can use file pickers and folder pickers.

![PathPikcers](https://user-images.githubusercontent.com/69258547/171440880-5948a7f0-5e26-4c38-ab95-8c6daaf67f93.png)

```json
{
    "gui": [
        {
            "label": "Copy file",
            "window_name": "Picker sample",
            "command": "copy %foo% %bar%",
            "button": "copy",
            "components": [
                {
                    "type": "file",
                    "label": "PNG file you want to copy",
                    "extension": "PNG files (*.png)|*.png",
                    "add_quotes": true
                },
                {
                    "type": "folder",
                    "label": "Output path",
                    "add_quotes": true,
                    "empty_message": "Drop a folder here!"
                }
            ]
        }
    ]
}
```

## "components"

`compoents` is an array of GUI components (e.g. file pickers).  
Each component should be defined as a dictionary.  

-   `type` is component type. `file` for a file picker, and `folder` for a dir picker.
-   `label` is a string written above the text box of picker.
-   `extension` is an optional key for file picker. It's wildcard for file pickers. Use the same syntax as for [wxWidget's wildcards](https://docs.wxwidgets.org/3.0/classwx_file_dialog.html).
-   `add_quotes` is an optional key for components. It will add quotes to the input strings.
-   `empty_message` is an optional key for text box. It will show a message when the text box is empty.

## "command"

Inputs of components will be injected into `command` when executing the command.  
You can specify where you inject the inputs with `%*%`.  
In the example, the file path will be injected in `%foo%`, and the folder path will be in `%bar%`.  
You can also use [`id`](../../comp_options/id) option to insert an input into multiple places.