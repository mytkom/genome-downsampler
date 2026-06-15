project = "Genome Downsampler"
author = "Marek Mytkowski, Borys Kurdek, Michał Okurowski"
copyright = "2026, Marek Mytkowski, Borys Kurdek, Michał Okurowski"

extensions = ["sphinx.ext.mathjax"]

templates_path = ["_templates"]
exclude_patterns = ["_build", "Thumbs.db", ".DS_Store"]

html_theme = "alabaster"
html_static_path = ["_static"]

html_theme_options = {
    "description": "Coverage-constrained paired-read downsampling",
    "github_button": True,
    "github_repo": "genome-downsampler",
    "github_user": "migoox",
}

html_title = project
