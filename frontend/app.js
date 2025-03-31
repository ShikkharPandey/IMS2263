document.addEventListener("DOMContentLoaded", function () {
    loadInventory();
});

// Function to load inventory and populate the table
function loadInventory() {
    fetch("/inventory")
        .then(response => response.json())
        .then(data => {
            const tableBody = document.getElementById("productTableBody");
            if (!tableBody) {
                console.error("Table body not found.");
                return;
            }
            tableBody.innerHTML = ""; // Clear any existing rows

            // Loop through the products and create rows
            data.forEach(item => {
                let row = document.createElement("tr");

                // Add table cells with data
                row.innerHTML = `
                    <td>${item.id}</td>
                    <td>${item.name}</td>
                    <td>${item.quantity}</td>
                    <td>$${item.price.toFixed(2)}</td>
                    <td>${item.upc || ""}</td>
                    <td>${item.sku || ""}</td>
                    <td>${item.department || ""}</td>
                    <td>${item.vendor || ""}</td>
                    <td class='actions'>
                        <button class="edit-btn" onclick="editItem(${item.id})">Edit</button>
                        <button class="delete-btn" onclick="deleteItem(${item.id})">Delete</button>
                    </td>
                `;

                // Append the row to the table
                tableBody.appendChild(row);
            });
        })
        .catch(error => console.error("Error loading inventory:", error));
}

// Function to delete a product
function deleteItem(itemId) {
    fetch(`/inventory?id=${itemId}`, {
        method: "DELETE"
    })
    .then(() => {
        loadInventory();  // Reload inventory after delete
        alert("Product deleted successfully!");
    })
    .catch(error => console.error("Error deleting item:", error));
}

// Function to edit a product
function editItem(itemId) {
    fetch("/inventory")
      .then(response => response.json())
      .then(data => {
        const item = data.find(i => i.id === itemId);
        if (!item) {
            alert("Item not found!");
            return;
        }

        // Using prompt() for editing (this can be replaced with a modal for better UX)
        const updatedItem = {
          id: item.id,
          name: prompt("Enter new name:", item.name) || item.name,
          quantity: parseInt(prompt("Enter new quantity:", item.quantity)) || item.quantity,
          price: parseFloat(prompt("Enter new price:", item.price)) || item.price,
          upc: prompt("Enter new UPC:", item.upc || "") || item.upc || "",
          sku: prompt("Enter new SKU:", item.sku || "") || item.sku || "",
          department: prompt("Enter new department:", item.department || "") || item.department || "",
          vendor: prompt("Enter new vendor:", item.vendor || "") || item.vendor || ""
        };
  
        fetch("/inventory", {
          method: "PUT",
          headers: { "Content-Type": "application/json" },
          body: JSON.stringify(updatedItem)
        })
        .then(response => response.json())
        .then(() => {
            loadInventory();  // Reload inventory after update
            alert("Product updated successfully!");
        })
        .catch(err => {
          console.error("Error updating item:", err);
          alert("Failed to update item.");
        });
      })
      .catch(err => {
        console.error("Error fetching inventory for editing:", err);
        alert("Failed to fetch inventory for editing.");
      });
}
