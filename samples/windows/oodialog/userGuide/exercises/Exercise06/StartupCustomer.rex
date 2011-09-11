/* Standalone startup for CustomerView. */

.local~my.idCustomerData  = .CustomerData~new	-- create Customer Data instance
.local~my.idCustomerModel = .CustomerModel~new	-- create Customer Model instance
.local~my.idCustomerData~activate
.local~my.idCustomerModel~activate

.CustomerView~newInstance("SA")

::REQUIRES "Customer\CustomerView.rex"
::REQUIRES "Customer\CustomerModelData.rex"