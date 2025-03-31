document.getElementById("add-form").addEventListener("submit", function (e) {
  e.preventDefault();

  const product = {
      name: document.getElementById("name").value.trim(),
      price: parseFloat(document.getElementById("price").value),
      quantity: parseInt(document.getElementById("quantity").value),
      upc: document.getElementById("upc").value.trim(),
      sku: document.getElementById("sku").value.trim(),
      department: document.getElementById("department").value.trim(),
      vendor: document.getElementById("vendor").value.trim(),
  };

  // Log the product data to confirm it's formatted correctly
  console.log("Product data being sent:", product);

  // Ensure that price and quantity are valid numbers
  if (isNaN(product.price) || isNaN(product.quantity)) {
      alert("Please provide valid numbers for price and quantity.");
      return;
  }

  // Send the data to the server
  fetch("/inventory", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify(product)
  })
  .then(res => res.json())
  .then(() => {
      document.getElementById("status-msg").textContent = "✅ Product added successfully!";
      document.getElementById("add-form").reset();
  })
  .catch(err => {
      document.getElementById("status-msg").textContent = "❌ Error adding product.";
      console.error(err);
  });
});
