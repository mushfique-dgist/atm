# Docs

This folder contains internal documentation for the DGIST ATM Simulator project.

- `project-uml.puml`: PlantUML source file describing the class diagram of the project.

How to generate the diagram:

1) Using PlantUML (requires Java):

```powershell
# If you have plantuml installed as a CLI:
plantuml -tsvg docs/project-uml.puml  # generates SVG
plantuml -tpng docs/project-uml.puml  # generates PNG

# Or with the JAR directly:
java -jar plantuml.jar -tsvg docs/project-uml.puml
java -jar plantuml.jar -tpng docs/project-uml.puml
```

2) Using VS Code extensions: install "PlantUML" and open `docs/project-uml.puml` to preview/export.

Notes:
- This file is intentionally created under `docs/` so it's not excluded by `.gitignore`.
- Feel free to modify the UML if you want additional method names, signatures, or relationships included.
