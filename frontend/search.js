document.getElementById("searchButton").addEventListener("click", function() {
    const query = document.getElementById("searchQuery").value.trim();
    if (!query) return;

    fetch(`/search?query=${encodeURIComponent(query)}`)
        .then(response => response.json())
        .then(data => {
            const resultsDiv = document.getElementById("searchTableBody");
            resultsDiv.innerHTML = '';
            
            if (data.length > 0) {
                data.forEach(item => {
                    const row = document.createElement("tr");
                    row.innerHTML = `
                        <td>${item.id}</td>
                        <td>${item.name}</td>
                    `;
                    resultsDiv.appendChild(row);
                });
            } else {
                resultsDiv.innerHTML = '<tr><td colspan="8">No results found.</td></tr>';
            }
        })
        .catch(err => console.error("Error with search:", err));
});
